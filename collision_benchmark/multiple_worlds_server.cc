/*
 * Copyright (C) 2012-2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <collision_benchmark/GazeboWorldLoader.hh>
#include <collision_benchmark/PhysicsWorld.hh>
#include <collision_benchmark/GazeboPhysicsWorld.hh>
#include <collision_benchmark/GazeboWorldState.hh>
#include <collision_benchmark/GazeboTopicForwardingMirror.hh>
#include <collision_benchmark/GazeboPhysicsWorld.hh>
#include <collision_benchmark/boost_std_conversion.hh>
#include <collision_benchmark/GazeboHelpers.hh>
#include <collision_benchmark/WorldManager.hh>
#include <collision_benchmark/GazeboControlServer.hh>

#include <collision_benchmark/GazeboMultipleWorldsServer.hh>
#include <collision_benchmark/WorldLoader.hh>

#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/sensors/SensorsIface.hh>

#include <boost/program_options.hpp>
#include <atomic>

using collision_benchmark::PhysicsWorldBaseInterface;
using collision_benchmark::PhysicsWorldStateInterface;
using collision_benchmark::PhysicsWorld;
using collision_benchmark::GazeboPhysicsWorld;
using collision_benchmark::GazeboPhysicsWorldTypes;
using collision_benchmark::MirrorWorld;
using collision_benchmark::GazeboTopicForwardingMirror;
using collision_benchmark::WorldManager;
using collision_benchmark::GazeboControlServer;

using collision_benchmark::WorldLoader;
using collision_benchmark::GazeboWorldLoader;
using collision_benchmark::MultipleWorldsServer;
using collision_benchmark::GazeboMultipleWorldsServer;

namespace po = boost::program_options;

typedef MultipleWorldsServer<GazeboPhysicsWorldTypes::WorldState,
                             GazeboPhysicsWorldTypes::ModelID,
                             GazeboPhysicsWorldTypes::ModelPartID,
                             GazeboPhysicsWorldTypes::Vector3,
                             GazeboPhysicsWorldTypes::Wrench>
                                GzMultipleWorldsServer;

typedef WorldManager<GazeboPhysicsWorldTypes::WorldState,
                     GazeboPhysicsWorldTypes::ModelID,
                     GazeboPhysicsWorldTypes::ModelPartID,
                     GazeboPhysicsWorldTypes::Vector3,
                     GazeboPhysicsWorldTypes::Wrench>
          GzWorldManager;

// test is paused or not
std::atomic<bool> g_unpaused(false);
std::atomic<bool> g_keypressed(false);


// the server
GzMultipleWorldsServer::Ptr g_server;

// waits until enter has been pressed and sets g_keypressed to true
void WaitForEnter()
{
  int key = getchar();
  g_keypressed=true;
}

// waits for either g_unpaused is set to
// true or until enter key was pressed.
void WaitForUnpause()
{
  g_keypressed = false;
  std::thread * t = new std::thread(WaitForEnter);
  t->detach();  // detach so it can be terminated
  while (!g_unpaused && !g_keypressed)
  {
    gazebo::common::Time::MSleep(100);
  }
  delete t;
}

void pauseCallback(bool pause)
{
  //std::cout<<"############ Pause callback: "<<pause<<std::endl;
  g_unpaused = !pause;
}

// will be called at each loop iteration
void LoopIter(int iter)
{
/*
  GzWorldManager::Ptr worldManager = g_server->GetWorldManager();
  PhysicsWorldBaseInterface::Ptr mirroredWorld
      = worldManager.GetMirroredWorld();
  GzPhysicsWorld::Ptr mirroredWorldCast =
      std::dynamic_pointer_cast<GzPhysicsWorld>(mirroredWorld);
  assert(mirroredWorldCast);
  std::vector<GzPhysicsWorld::ContactInfoPtr> contacts =
      mirroredWorldCast->GetContactInfo();
  std::cout<<"Number of contacts: "<<contacts.size()<<std::endl;
  getchar();*/

  // temporary for testing: when paused, idle a little,
  // to not make this print too often
  /*const static int printCnt = 100;
  static int print = printCnt;
  if (!g_unpaused)
    gazebo::common::Time::MSleep(100);
  if (print <= 0)
  {
    std::cout<<"Updating world still going."<<std::endl;
    print=printCnt;
  }
  --print;*/
}

// Initializes the multiple worlds server
bool Init(const bool loadMirror,
          const bool allowControlViaMirror,
          const bool enforceContactCalc)
{
  std::set<std::string> engines =
    collision_benchmark::GetSupportedPhysicsEngines();
  GzMultipleWorldsServer::WorldLoader_M loaders;
  for (std::set<std::string>::const_iterator
       it = engines.begin(); it != engines.end(); ++it)
  {
    std::string engine = *it;
    try
    {
      loaders[engine] =
        WorldLoader::ConstPtr(new GazeboWorldLoader(engine,
                                                    enforceContactCalc));
    }
    catch (collision_benchmark::Exception& e)
    {
      std::cerr << "Could not add support for engine "
                <<engine << ": " << e.what() << std::endl;
      continue;
    }
  }

  if (loaders.empty())
  {
    std::cerr << "Could not get support for any engine." << std::endl;
    return false;
  }

  WorldLoader::Ptr universalLoader(new GazeboWorldLoader(enforceContactCalc));

  g_server.reset(new GazeboMultipleWorldsServer(loaders, universalLoader));

  int argc = 1;
  const char * argv = "MultipleWorldsServer";
  g_server->Start(argc, &argv);

  std::string mirrorName = "";
  if (loadMirror) mirrorName = "mirror";

  g_server->Init(mirrorName, allowControlViaMirror);

  GzWorldManager::Ptr worldManager = g_server->GetWorldManager();
  if (!worldManager) return false;
  return true;
}


// Runs the multiple worlds server
bool Run()
{
  GzWorldManager::Ptr worldManager = g_server->GetWorldManager();
  if (!worldManager) return false;

  GzWorldManager::ControlServerPtr controlServer =
    worldManager->GetControlServer();

  if (controlServer)
  {
    controlServer->RegisterPauseCallback(std::bind(pauseCallback,
                                                   std::placeholders::_1));
  }

//  worldManager->SetDynamicsEnabled(false);
  worldManager->SetPaused(true);

  std::cout << "Now start gzclient if you would like "
            << "to view the test: "<<std::endl;
  std::cout << "gzclient --g libcollision_benchmark_gui.so" << std::endl;
  std::cout << "Press [Enter] to continue without gzclient or hit "
            << "the play button in gzclient."<<std::endl;
  WaitForUnpause();

  worldManager->SetPaused(false);

  std::cout << "Now starting to update worlds."<<std::endl;
  int iter = 0;
  while(true)
  {
    int numSteps=1;
    worldManager->Update(numSteps);
    LoopIter(iter);
    ++iter;
  }
  g_server->Stop();
  return true;
}


/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  std::vector<std::string> selectedEngines;
  std::vector<std::string> worldFiles;

  // description for engine options as stream so line doesn't go over 80 chars.
  std::stringstream descEngines;
  descEngines <<  "Specify one or several physics engines. " <<
      "Can contain [ode, bullet, dart, simbody]. When not specified, worlds " <<
      "are loaded with the engine specified in the file. If specified, all " <<
      "worlds are loaded with each of the engines specified.";

  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "Produce help message")
    ("engines,e",
      po::value<std::vector<std::string>>(&selectedEngines)->multitoken(),
      descEngines.str().c_str())
    ("keep-name,k", "keep the names of the worlds as specified in the files. \
Only works when no engines are specified with -e.")
    ;
  po::options_description desc_hidden("Positional options");
  desc_hidden.add_options()
    ("worlds,w", po::value<std::vector<std::string>>(&worldFiles)->multitoken(),
      "World file(s).")
    ;

  po::variables_map vm;

  po::positional_options_description p;
  // positional arguments default to "worlds" argument
  p.add("worlds", -1);

  po::options_description desc_composite;
  desc_composite.add(desc).add(desc_hidden);

  po::command_line_parser parser{argc, argv};
  parser.options(desc_composite).positional(p); // .allow_unregistered();
  po::parsed_options parsedOpt = parser.run();
  po::store(parsedOpt, vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << argv[0] <<" <list of world files> " << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("engines"))
  {
    std::cout << "Engines to load: " << std::endl;
    for (std::vector<std::string>::iterator it = selectedEngines.begin();
         it != selectedEngines.end(); ++it)
    {
      std::cout<<*it<<std::endl;
    }
  }
  else
  {
    std::cout << "No engines were given, so using physics information "
              << "specified in world files" << std::endl;
  }

  if (!vm.count("worlds"))
  {
    std::cout << "You need to specify at least one world." << std::endl;
    return 0;
  }

  // Initialize server
  bool loadMirror = true;
  bool enforceContactCalc=false;
  bool allowControlViaMirror = true;
  Init(loadMirror, allowControlViaMirror, enforceContactCalc);
  assert(g_server);

  // load the worlds as given in command line arguments
  // with the engine names given
  int i = 0;
  for (std::vector<std::string>::iterator it = worldFiles.begin();
       it != worldFiles.end(); ++it, ++i)
  {
    std::string worldfile = *it;
    std::cout<<"Loading world " << worldfile <<std::endl;

    std::string worldPrefix;

    if (!selectedEngines.empty() || !vm.count("keep-name"))
    {
      std::stringstream _worldPrefix;
      _worldPrefix << "world" << "_" << i;
      worldPrefix = _worldPrefix.str();
    }

    if (selectedEngines.empty())
    {
      if (g_server->AutoLoad(worldfile, worldPrefix) < 0)
        std::cerr << "Could not auto-load world " << worldfile << std::endl;
    }
    else
    {
      g_server->Load(worldfile, selectedEngines, worldPrefix);
    }
  }

  Run();
}
