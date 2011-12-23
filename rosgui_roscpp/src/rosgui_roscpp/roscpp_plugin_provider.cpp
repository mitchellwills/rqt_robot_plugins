/*
 * Copyright (c) 2011, Dirk Thomas, TU Darmstadt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the TU Darmstadt nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "roscpp_plugin_provider.h"

#include "nodelet_plugin_provider.h"
#include <rosgui_roscpp/plugin.h>

#include <rosgui_cpp/plugin_provider.h>

#include <nodelet/nodelet.h>
#include <pluginlib/class_list_macros.h>
#include <ros/ros.h>

#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

namespace rosgui_roscpp {

RosCppPluginProvider::RosCppPluginProvider()
  : rosgui_cpp::CompositePluginProvider()
  , manager_name_("rosgui_roscpp_node")
  , callback_manager_(0)
  , node_initialized_(false)
{
  QList<PluginProvider*> plugin_providers;
  plugin_providers.append(new NodeletPluginProvider("rosgui", "rosgui_roscpp::Plugin", this));
  set_plugin_providers(plugin_providers);
}

RosCppPluginProvider::~RosCppPluginProvider()
{
  if (callback_manager_)
  {
    delete callback_manager_;
  }

  if (ros::isStarted())
  {
    ros::shutdown();
  }
}

void* RosCppPluginProvider::load(const QString& plugin_id, rosgui_cpp::PluginContext* plugin_context)
{
  init_node();
  return rosgui_cpp::CompositePluginProvider::load(plugin_id, plugin_context);
}

rosgui_cpp::Plugin* RosCppPluginProvider::load_plugin(const QString& plugin_id, rosgui_cpp::PluginContext* plugin_context)
{
  init_node();
  return rosgui_cpp::CompositePluginProvider::load_plugin(plugin_id, plugin_context);
}

void RosCppPluginProvider::init_node()
{
  // initialize ROS nodelet manager once
  if (!node_initialized_)
  {
    int argc = 0;
    char** argv = 0;
    std::stringstream name;
    name << "rosgui_roscpp_node_";
    name << getpid();
    qDebug("RosCppPluginProvider::init_node() initialize ROS node '%s'", name.str().c_str());
    ros::init(argc, argv, name.str().c_str(), ros::init_options::NoSigintHandler);
    if (!ros::master::check())
    {
      throw std::runtime_error("RosCppPluginProvider::init_node() could not find ROS master");
    }
    ros::start();
    callback_manager_ = new nodelet::detail::CallbackQueueManager();
    ros::NodeHandle nh(manager_name_);
    node_initialized_ = true;
  }
}

}

PLUGINLIB_DECLARE_CLASS(rosgui_roscpp, RosCppPluginProvider, rosgui_roscpp::RosCppPluginProvider, rosgui_cpp::PluginProvider)
