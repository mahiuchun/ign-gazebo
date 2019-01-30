/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gtest/gtest.h>

#include <ignition/msgs/entity_factory.pb.h>

#include <ignition/common/Console.hh>
#include <ignition/math/Pose3.hh>
#include <ignition/transport/Node.hh>

#include "ignition/gazebo/components/Light.hh"
#include "ignition/gazebo/components/Model.hh"
#include "ignition/gazebo/components/Name.hh"
#include "ignition/gazebo/components/Pose.hh"
#include "ignition/gazebo/Server.hh"
#include "ignition/gazebo/SystemLoader.hh"
#include "ignition/gazebo/test_config.hh"

#include "plugins/MockSystem.hh"

using namespace ignition;
using namespace gazebo;

//////////////////////////////////////////////////
class UserCommandsTest : public ::testing::Test
{
  // Documentation inherited
  protected: void SetUp() override
  {
    ignition::common::Console::SetVerbosity(4);
    setenv("IGN_GAZEBO_SYSTEM_PLUGIN_PATH",
           (std::string(PROJECT_BINARY_PATH) + "/lib").c_str(), 1);
  }
};

//////////////////////////////////////////////////
class Relay
{
  public: Relay()
  {
    auto plugin = loader.LoadPlugin("libMockSystem.so",
                                    "ignition::gazebo::MockSystem",
                                    nullptr);
    EXPECT_TRUE(plugin.has_value());

    this->systemPtr = plugin.value();

    this->mockSystem =
        dynamic_cast<MockSystem *>(systemPtr->QueryInterface<System>());
    EXPECT_NE(nullptr, this->mockSystem);
  }

  public: Relay &OnPreUpdate(MockSystem::CallbackType _cb)
  {
    this->mockSystem->preUpdateCallback = std::move(_cb);
    return *this;
  }

  public: Relay &OnUpdate(MockSystem::CallbackType _cb)
  {
    this->mockSystem->updateCallback = std::move(_cb);
    return *this;
  }

  public: Relay &OnPostUpdate(MockSystem::CallbackTypeConst _cb)
  {
    this->mockSystem->postUpdateCallback = std::move(_cb);
    return *this;
  }

  public: SystemPluginPtr systemPtr;

  private: SystemLoader loader;
  private: MockSystem *mockSystem;
};

/////////////////////////////////////////////////
TEST_F(UserCommandsTest, Factory)
{
  // Start server
  ServerConfig serverConfig;
  const auto sdfFile = std::string(PROJECT_SOURCE_PATH) +
    "/examples/worlds/empty.sdf";
  serverConfig.SetSdfFile(sdfFile);

  Server server(serverConfig);
  EXPECT_FALSE(server.Running());
  EXPECT_FALSE(*server.Running(0));

  // Create a system just to get the ECM
  // TODO(louise) It would be much more convenient if the Server just returned
  // the ECM for us. This would save all the trouble which is causing us to
  // create `Relay` systems in the first place. Consider keeping the ECM in a
  // shared pointer owned by the SimulationRunner.
  EntityComponentManager *ecm{nullptr};
  Relay testSystem;
  testSystem.OnPreUpdate([&](const gazebo::UpdateInfo &,
                             gazebo::EntityComponentManager &_ecm)
      {
        ecm = &_ecm;
      });

  server.AddSystem(testSystem.systemPtr);

  // Run server and check we have the ECM
  EXPECT_EQ(nullptr, ecm);
  server.Run(true, 1, false);
  EXPECT_NE(nullptr, ecm);

  auto entityCount = ecm->EntityCount();

  // SDF strings
  auto modelStr = std::string("<?xml version=\"1.0\" ?>") +
      "<sdf version=\"1.6\">" +
      "<model name=\"spawned_model\">" +
      "<link name=\"link\">" +
      "<visual name=\"visual\">" +
      "<geometry><sphere><radius>1.0</radius></sphere></geometry>" +
      "</visual>" +
      "<collision name=\"visual\">" +
      "<geometry><sphere><radius>1.0</radius></sphere></geometry>" +
      "</collision>" +
      "</link>" +
      "</model>" +
      "</sdf>";

  auto lightStr = std::string("<?xml version=\"1.0\" ?>") +
      "<sdf version=\"1.6\">" +
      "<light name=\"spawned_light\" type=\"directional\">" +
      "</light>" +
      "</sdf>";

  // Request entity spawn
  msgs::EntityFactory req;
  req.set_sdf(modelStr);

  auto pose = req.mutable_pose();
  auto pos = pose->mutable_position();
  pos->set_z(10);

  msgs::Boolean res;
  bool result;
  unsigned int timeout = 5000;
  std::string service{"/world/empty/factory"};

  transport::Node node;
  EXPECT_TRUE(node.Request(service, req, timeout, res, result));
  EXPECT_TRUE(result);
  EXPECT_TRUE(res.data());

  // Check entity has not been created yet
  EXPECT_EQ(kNullEntity, ecm->EntityByComponents(components::Model(),
      components::Name("spawned_model")));

  // Run an iteration and check it was created
  server.Run(true, 1, false);
  EXPECT_LT(entityCount, ecm->EntityCount());
  entityCount = ecm->EntityCount();

  auto model = ecm->EntityByComponents(components::Model(),
      components::Name("spawned_model"));
  EXPECT_NE(kNullEntity, model);

  auto poseComp = ecm->Component<components::Pose>(model);
  EXPECT_NE(nullptr, poseComp);

  EXPECT_EQ(math::Pose3d(0, 0, 10, 0, 0, 0), poseComp->Data());

  // Request to spawn same model and check if fails due to repeated name
  req.Clear();
  req.set_sdf(modelStr);

  EXPECT_TRUE(node.Request(service, req, timeout, res, result));
  EXPECT_TRUE(result);
  EXPECT_TRUE(res.data());

  // Run an iteration and check it was not created
  server.Run(true, 1, false);

  EXPECT_EQ(entityCount, ecm->EntityCount());

  // Enable renaming and check it is spawned with new name
  req.Clear();
  req.set_sdf(modelStr);
  req.set_allow_renaming(true);

  EXPECT_TRUE(node.Request(service, req, timeout, res, result));
  EXPECT_TRUE(result);
  EXPECT_TRUE(res.data());

  // Run an iteration and check it was created with a new name
  server.Run(true, 1, false);

  EXPECT_LT(entityCount, ecm->EntityCount());
  entityCount = ecm->EntityCount();

  model = ecm->EntityByComponents(components::Model(),
      components::Name("spawned_model_0"));
  EXPECT_NE(kNullEntity, model);

  // Spawn with a different name
  req.Clear();
  req.set_sdf(modelStr);
  req.set_name("banana");

  EXPECT_TRUE(node.Request(service, req, timeout, res, result));
  EXPECT_TRUE(result);
  EXPECT_TRUE(res.data());

  // Run an iteration and check it was created with given name
  server.Run(true, 1, false);

  EXPECT_LT(entityCount, ecm->EntityCount());
  entityCount = ecm->EntityCount();

  model = ecm->EntityByComponents(components::Model(),
      components::Name("banana"));
  EXPECT_NE(kNullEntity, model);

  // Spawn a light
  req.Clear();
  req.set_sdf(lightStr);

  EXPECT_TRUE(node.Request(service, req, timeout, res, result));
  EXPECT_TRUE(result);
  EXPECT_TRUE(res.data());

  // Run an iteration and check it was created
  server.Run(true, 1, false);

  EXPECT_LT(entityCount, ecm->EntityCount());
  entityCount = ecm->EntityCount();

  auto light = ecm->EntityByComponents(components::Name("spawned_light"));
  EXPECT_NE(kNullEntity, light);

  EXPECT_NE(nullptr, ecm->Component<components::Light>(light));

  // Queue commands and check they're all executed in the same iteration
  req.Clear();
  req.set_sdf(modelStr);
  req.set_name("acerola");

  EXPECT_TRUE(node.Request(service, req, timeout, res, result));
  EXPECT_TRUE(result);
  EXPECT_TRUE(res.data());

  req.Clear();
  req.set_sdf(modelStr);
  req.set_name("coconut");

  EXPECT_TRUE(node.Request(service, req, timeout, res, result));
  EXPECT_TRUE(result);
  EXPECT_TRUE(res.data());

  // Check neither exists yet
  EXPECT_EQ(kNullEntity, ecm->EntityByComponents(components::Model(),
      components::Name("acerola")));
  EXPECT_EQ(kNullEntity, ecm->EntityByComponents(components::Model(),
      components::Name("coconut")));
  EXPECT_EQ(entityCount, ecm->EntityCount());

  // Run an iteration and check both models were created
  server.Run(true, 1, false);

  EXPECT_LT(entityCount, ecm->EntityCount());
  entityCount = ecm->EntityCount();

  EXPECT_NE(kNullEntity, ecm->EntityByComponents(components::Model(),
      components::Name("acerola")));
  EXPECT_NE(kNullEntity, ecm->EntityByComponents(components::Model(),
      components::Name("coconut")));
}