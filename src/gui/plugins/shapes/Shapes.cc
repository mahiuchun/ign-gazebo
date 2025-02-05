/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include "Shapes.hh"

#include <ignition/msgs/boolean.pb.h>
#include <ignition/msgs/stringmsg.pb.h>

#include <algorithm>
#include <iostream>
#include <string>

#include <ignition/common/Console.hh>
#include <ignition/gui/Application.hh>
#include <ignition/gui/GuiEvents.hh>
#include <ignition/gui/MainWindow.hh>
#include <ignition/plugin/Register.hh>
#include <ignition/transport/Node.hh>
#include <ignition/transport/Publisher.hh>
#include <ignition/utils/SuppressWarning.hh>

#include "ignition/gazebo/gui/GuiEvents.hh"

namespace ignition::gazebo
{
  class ShapesPrivate
  {
    /// \brief Ignition communication node.
    public: transport::Node node;

    /// \brief Transform control service name
    public: std::string service;
  };
}

using namespace ignition;
using namespace gazebo;

/////////////////////////////////////////////////
Shapes::Shapes()
  : ignition::gui::Plugin(),
  dataPtr(std::make_unique<ShapesPrivate>())
{
}

/////////////////////////////////////////////////
Shapes::~Shapes() = default;

/////////////////////////////////////////////////
void Shapes::LoadConfig(const tinyxml2::XMLElement *)
{
  if (this->title.empty())
    this->title = "Shapes";

  // For shapes requests
  ignition::gui::App()->findChild
    <ignition::gui::MainWindow *>()->installEventFilter(this);
}

/////////////////////////////////////////////////
void Shapes::OnMode(const QString &_mode)
{
  std::string modelSdfString = _mode.toStdString();
  std::transform(modelSdfString.begin(), modelSdfString.end(),
                 modelSdfString.begin(), ::tolower);

  if (modelSdfString == "box")
  {
    modelSdfString = std::string("<?xml version=\"1.0\"?>"
                                 "<sdf version=\"1.8\">"
                                   "<model name=\"box\">"
                                     "<pose>0 0 0.5 0 0 0</pose>"
                                     "<link name=\"box_link\">"
                                       "<inertial>"
                                         "<inertia>"
                                           "<ixx>0.16666</ixx>"
                                           "<ixy>0</ixy>"
                                           "<ixz>0</ixz>"
                                           "<iyy>0.16666</iyy>"
                                           "<iyz>0</iyz>"
                                           "<izz>0.16666</izz>"
                                         "</inertia>"
                                         "<mass>1.0</mass>"
                                       "</inertial>"
                                       "<collision name=\"box_collision\">"
                                         "<geometry>"
                                           "<box>"
                                             "<size>1 1 1</size>"
                                           "</box>"
                                         "</geometry>"
                                       "</collision>"
                                       "<visual name=\"box_visual\">"
                                         "<geometry>"
                                           "<box>"
                                             "<size>1 1 1</size>"
                                           "</box>"
                                         "</geometry>"
                                       "</visual>"
                                     "</link>"
                                   "</model>"
                                 "</sdf>");
  }
  else if (modelSdfString == "sphere")
  {
    modelSdfString = std::string("<?xml version=\"1.0\"?>"
                                 "<sdf version=\"1.8\">"
                                   "<model name=\"sphere\">"
                                     "<pose>0 0 0.5 0 0 0</pose>"
                                     "<link name=\"sphere_link\">"
                                       "<inertial>"
                                         "<inertia>"
                                           "<ixx>0.1</ixx>"
                                           "<ixy>0</ixy>"
                                           "<ixz>0</ixz>"
                                           "<iyy>0.1</iyy>"
                                           "<iyz>0</iyz>"
                                           "<izz>0.1</izz>"
                                         "</inertia>"
                                         "<mass>1.0</mass>"
                                       "</inertial>"
                                       "<collision name=\"sphere_collision\">"
                                         "<geometry>"
                                           "<sphere>"
                                             "<radius>0.5</radius>"
                                           "</sphere>"
                                         "</geometry>"
                                       "</collision>"
                                       "<visual name=\"sphere_visual\">"
                                         "<geometry>"
                                           "<sphere>"
                                             "<radius>0.5</radius>"
                                           "</sphere>"
                                         "</geometry>"
                                       "</visual>"
                                     "</link>"
                                   "</model>"
                                 "</sdf>");
  }
  else if (modelSdfString == "cylinder")
  {
    modelSdfString = std::string("<?xml version=\"1.0\"?>"
                                 "<sdf version=\"1.8\">"
                                   "<model name=\"cylinder\">"
                                     "<pose>0 0 0.5 0 0 0</pose>"
                                     "<link name=\"cylinder_link\">"
                                       "<inertial>"
                                         "<inertia>"
                                           "<ixx>0.1458</ixx>"
                                           "<ixy>0</ixy>"
                                           "<ixz>0</ixz>"
                                           "<iyy>0.1458</iyy>"
                                           "<iyz>0</iyz>"
                                           "<izz>0.125</izz>"
                                         "</inertia>"
                                         "<mass>1.0</mass>"
                                       "</inertial>"
                                       "<collision name=\"cylinder_collision\">"
                                         "<geometry>"
                                           "<cylinder>"
                                             "<radius>0.5</radius>"
                                             "<length>1.0</length>"
                                           "</cylinder>"
                                         "</geometry>"
                                       "</collision>"
                                       "<visual name=\"cylinder_visual\">"
                                         "<geometry>"
                                           "<cylinder>"
                                             "<radius>0.5</radius>"
                                             "<length>1.0</length>"
                                           "</cylinder>"
                                         "</geometry>"
                                       "</visual>"
                                     "</link>"
                                   "</model>"
                                 "</sdf>");
  }
  else if (modelSdfString == "capsule")
  {
    modelSdfString = std::string("<?xml version=\"1.0\"?>"
                                 "<sdf version=\"1.8\">"
                                   "<model name=\"capsule\">"
                                     "<pose>0 0 0.5 0 0 0</pose>"
                                     "<link name=\"capsule_link\">"
                                       "<inertial>"
                                         "<inertia>"
                                           "<ixx>0.074154</ixx>"
                                           "<ixy>0</ixy>"
                                           "<ixz>0</ixz>"
                                           "<iyy>0.074154</iyy>"
                                           "<iyz>0</iyz>"
                                           "<izz>0.018769</izz>"
                                         "</inertia>"
                                         "<mass>1.0</mass>"
                                       "</inertial>"
                                       "<collision name=\"capsule_collision\">"
                                         "<geometry>"
                                           "<capsule>"
                                             "<radius>0.2</radius>"
                                             "<length>0.6</length>"
                                           "</capsule>"
                                         "</geometry>"
                                       "</collision>"
                                       "<visual name=\"capsule_visual\">"
                                         "<geometry>"
                                           "<capsule>"
                                             "<radius>0.2</radius>"
                                             "<length>0.6</length>"
                                           "</capsule>"
                                         "</geometry>"
                                       "</visual>"
                                     "</link>"
                                   "</model>"
                                 "</sdf>");
  }
  else if (modelSdfString == "ellipsoid")
  {
    modelSdfString = std::string("<?xml version=\"1.0\"?>"
                                 "<sdf version=\"1.8\">"
                                   "<model name=\"ellipsoid\">"
                                     "<pose>0 0 0.5 0 0 0</pose>"
                                     "<link name=\"ellipsoid_link\">"
                                       "<inertial>"
                                         "<inertia>"
                                           "<ixx>0.068</ixx>"
                                           "<ixy>0</ixy>"
                                           "<ixz>0</ixz>"
                                           "<iyy>0.058</iyy>"
                                           "<iyz>0</iyz>"
                                           "<izz>0.026</izz>"
                                         "</inertia>"
                                         "<mass>1.0</mass>"
                                       "</inertial>"
                                     "<collision name=\"ellipsoid_collision\">"
                                         "<geometry>"
                                           "<ellipsoid>"
                                             "<radii>0.2 0.3 0.5</radii>"
                                           "</ellipsoid>"
                                         "</geometry>"
                                       "</collision>"
                                       "<visual name=\"ellipsoid_visual\">"
                                         "<geometry>"
                                           "<ellipsoid>"
                                             "<radii>0.2 0.3 0.5</radii>"
                                           "</ellipsoid>"
                                         "</geometry>"
                                       "</visual>"
                                     "</link>"
                                   "</model>"
                                 "</sdf>");
  }
  else
  {
    ignwarn << "Invalid model string " << modelSdfString << "\n";
    ignwarn << "The valid options are:\n";
    ignwarn << " - box\n";
    ignwarn << " - sphere\n";
    ignwarn << " - capsule\n";
    ignwarn << " - cylinder\n";
    ignwarn << " - ellipsoid\n";
    return;
  }

  ignition::gui::events::SpawnFromDescription event(modelSdfString);
  ignition::gui::App()->sendEvent(
      ignition::gui::App()->findChild<ignition::gui::MainWindow *>(),
      &event);

  IGN_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  ignition::gazebo::gui::events::SpawnPreviewModel oldEvent(modelSdfString);
  ignition::gui::App()->sendEvent(
      ignition::gui::App()->findChild<ignition::gui::MainWindow *>(),
      &oldEvent);
  IGN_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gazebo::Shapes,
                    ignition::gui::Plugin)
