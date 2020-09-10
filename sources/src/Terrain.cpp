/*
MIT License

Copyright (c) 2020 Andrey Vasiliev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "pcheader.h"
#include "Terrain.h"
#include "Physics.h"
#include "TerrainMaterialGeneratorB.h"
#include "PbrUtils.h"
#include "XmlUtils.h"

namespace xio {
Terrain::Terrain() {}

Terrain::~Terrain() {
  if (terrain_) {
    terrain_->removeAllTerrains();
    terrain_.reset();
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Terrain::GetTerrainImage_(bool flipX,
                                       bool flipY,
                                       Ogre::Image &ogre_image,
                                       const std::string &filename = "terrain.dds") {
  ogre_image.load(filename, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

  if (flipX)
    ogre_image.flipAroundY();

  if (flipY)
    ogre_image.flipAroundX();
}
//----------------------------------------------------------------------------------------------------------------------
void Terrain::DefineTerrain_(long x, long y, bool flat, const std::string &filename = "terrain.dds") {
  // if a file is available, use it
  // if not, generate file from import

  // Usually in a real project you'll know whether the compact terrain data is
  // available or not; I'm doing it this way to save distribution size

  if (flat) {
    terrain_->defineTerrain(x, y, 0.0f);
    return;
  }

  Ogre::Image image;
  GetTerrainImage_(x % 2 != 0, y % 2 != 0, image, filename);

  std::string cached = terrain_->generateFilename(x, y);
  if (Ogre::ResourceGroupManager::getSingleton().resourceExists(terrain_->getResourceGroup(),
                                                                cached)) {
    terrain_->defineTerrain(x, y);
  } else {
    terrain_->defineTerrain(x, y, &image);
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Terrain::InitBlendMaps_(Ogre::Terrain *terrain,
                                     int layer,
                                     const std::string &image = "terrain_blendmap.dds") {
  Ogre::TerrainLayerBlendMap *blendMap = terrain->getLayerBlendMap(layer);
  auto *pBlend1 = blendMap->getBlendPointer();

  Ogre::Image img;
  img.load(image, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

  int bmSize = terrain->getLayerBlendMapSize();

  for (int y = 0; y < bmSize; ++y) {
    for (int x = 0; x < bmSize; ++x) {
      float tx = static_cast<float>(x) / static_cast<float>(bmSize);
      float ty = static_cast<float>(y) / static_cast<float>(bmSize);
      int ix = static_cast<int>(img.getWidth() * tx);
      int iy = static_cast<int >(img.getWidth() * ty);

      Ogre::ColourValue cv = Ogre::ColourValue::Black;
      cv = img.getColourAt(ix, iy, 0);
//            Ogre::Real val = cv[0]*opacity[bi];
      float val = cv[0];
      *pBlend1++ = val;
    }
  }

  blendMap->dirty();
  blendMap->update();

  // set up a colour map
  if (terrain->getGlobalColourMapEnabled()) {
    Ogre::Image colourMap;
    colourMap.load("testcolourmap.dds", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
    terrain->getGlobalColourMap()->loadImage(colourMap);
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Terrain::ProcessTerrainGroup(pugi::xml_node &xml_node) {
  float worldSize = GetAttribReal(xml_node, "worldSize");
  int mapSize = GetAttribInt(xml_node, "mapSize");
  int minBatchSize = Ogre::StringConverter::parseInt(xml_node.attribute("minBatchSize").value());
  int maxBatchSize = Ogre::StringConverter::parseInt(xml_node.attribute("maxBatchSize").value());
  int inputScale = Ogre::StringConverter::parseInt(xml_node.attribute("inputScale").value());
  int tuningCompositeMapDistance =
      Ogre::StringConverter::parseInt(xml_node.attribute("tuningCompositeMapDistance").value());
  int tuningMaxPixelError = GetAttribInt(xml_node, "tuningMaxPixelError", 8);
  auto *terrain_global_options = Ogre::TerrainGlobalOptions::getSingletonPtr();

  if (!terrain_global_options)
    terrain_global_options = new Ogre::TerrainGlobalOptions();

  OgreAssert(terrain_global_options, "Ogre::TerrainGlobalOptions not available");
  terrain_global_options->setMaxPixelError(static_cast<float>(tuningMaxPixelError));
  terrain_global_options->setCompositeMapDistance(static_cast<float>(tuningCompositeMapDistance));
  terrain_global_options->setCastsDynamicShadows(false);
  terrain_global_options->setUseRayBoxDistanceCalculation(false);
  terrain_global_options->setDefaultMaterialGenerator(std::make_shared<TerrainMaterialGeneratorB>());

  auto *scene_manager = Ogre::Root::getSingleton().getSceneManager("Default");
  terrain_ = std::make_unique<Ogre::TerrainGroup>(scene_manager, Ogre::Terrain::ALIGN_X_Z, mapSize, worldSize);
  terrain_->setFilenameConvention("terrain", "bin");
  terrain_->setOrigin(Ogre::Vector3::ZERO);
  terrain_->setResourceGroup(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  Ogre::Terrain::ImportData &defaultimp = terrain_->getDefaultImportSettings();

  defaultimp.terrainSize = mapSize;
  defaultimp.worldSize = worldSize;
  defaultimp.inputScale = inputScale;
  defaultimp.minBatchSize = minBatchSize;
  defaultimp.maxBatchSize = maxBatchSize;

  for (auto pPageElement : xml_node.children("terrain")) {
    int pageX = Ogre::StringConverter::parseInt(pPageElement.attribute("x").value());
    int pageY = Ogre::StringConverter::parseInt(pPageElement.attribute("y").value());

    std::string heighmap = pPageElement.attribute("heightmap").value();
    DefineTerrain_(pageX, pageY, false, heighmap);
    terrain_->loadTerrain(pageX, pageY, true);
    terrain_->getTerrain(pageX, pageY)->setGlobalColourMapEnabled(false);

    int layers_count = 0;
    for (const auto &pLayerElement : pPageElement.children("layer")) {
      layers_count++;
    }

    defaultimp.layerList.resize(layers_count);

    int layer_counter = 0;
    for (auto pLayerElement : pPageElement.children("layer")) {
      defaultimp.layerList[layer_counter].worldSize =
          Ogre::StringConverter::parseInt(pLayerElement.attribute("scale").value());
      defaultimp.layerList[layer_counter].textureNames.push_back(pLayerElement.attribute("diffuse").value());
      defaultimp.layerList[layer_counter].textureNames.push_back(pLayerElement.attribute("normal").value());

      layer_counter++;
    }

    layer_counter = 0;
    for (auto pLayerElement : pPageElement.children("layer")) {
      layer_counter++;
      if (layer_counter != layers_count) {
        InitBlendMaps_(terrain_->getTerrain(pageX, pageY),
                       layer_counter,
                       pLayerElement.attribute("blendmap").value());
      }

    }
  }

  terrain_->freeTemporaryResources();

  auto terrainIterator = terrain_->getTerrainIterator();

  while (terrainIterator.hasMoreElements()) {
    auto *terrain = terrainIterator.getNext()->instance;

    physics_->CreateTerrainHeightfieldShape(terrain->getSize(),
                                            terrain->getHeightData(),
                                            terrain->getMinHeight(),
                                            terrain->getMaxHeight(),
                                            terrain->getPosition(),
                                            terrain->getWorldSize() / (static_cast<float>(terrain->getSize() - 1))
    );
  }
}
//----------------------------------------------------------------------------------------------------------------------
float Terrain::GetHeigh(float x, float z) {
  if (terrain_)
    return terrain_->getHeightAtWorldPosition(x, 1000, z);
  else
    return 0.0f;
}
}