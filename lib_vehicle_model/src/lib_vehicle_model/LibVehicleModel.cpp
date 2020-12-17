/*
 * Copyright (C) 2018-2021 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <mutex>
#include <string>
#include <dlfcn.h>
#include <sstream>
#include "lib_vehicle_model/LibVehicleModel.h"
#include "lib_vehicle_model/ROSParameterServer.h"
#include "ModelLoader.h"
#include "ConstraintChecker.h"



/**
 * Cpp containing the implementation of lib_vehicle_model namespace public functions
 */
namespace lib_vehicle_model {

  //
  // Private Namespace
  //
  namespace {
    std::mutex init_mutex_; // Mutex for thread safety
    std::unique_ptr<VehicleMotionModel, ModelLoader::destroy_fnc_ptr> vehicle_model_(nullptr, nullptr);
    std::unique_ptr<ConstraintChecker> constraint_checker_;
    bool modelLoaded_ = false; // Flag indicating init has already been called
  }

  //
  // Public Namespace
  //
  void init(std::shared_ptr<ParameterServer> parameter_server) {

    // Mutex lock to ensure thread safety of lib loading and parameter loading
    // Since this function is the only place static data members are modified all other functions should be thread safe
    std::lock_guard<std::mutex> guard(init_mutex_); 

    if (modelLoaded_) {
      throw ModelAccessException("Attempted to load the vehicle model a second time from the same process");
    }

    // Load Parameters
    std::string vehicle_model_lib_path;
    bool pathParam = parameter_server->getParam("vehicle_model_lib_path", vehicle_model_lib_path);

    // Check if all the required parameters could be loaded
    if (!pathParam) {
      // Throw exception
      throw std::invalid_argument("The vehicle path param vehicle_model_lib_path could not be found or read");
    }

    constraint_checker_.reset(new ConstraintChecker(parameter_server));

    // Load the vehicle model to be used
    vehicle_model_ = ModelLoader::load(vehicle_model_lib_path);
    vehicle_model_->setParameterServer(parameter_server);

    // Set model loading flag
    modelLoaded_ = true;
  }

  void init(std::shared_ptr<ros::NodeHandle> nh) {
    std::shared_ptr<ROSParameterServer> ros_param_server = std::make_shared<ROSParameterServer>(nh);
    init(ros_param_server);
  }

  void unload() {
    // Lock mutex for thread safety
    std::lock_guard<std::mutex> guard(init_mutex_); 

    // Release allocated objects
    if (modelLoaded_) {
      vehicle_model_.reset(); // Release and call loaded lib destructor
      constraint_checker_.reset();
      modelLoaded_ = false;
    }
  }

  std::vector<VehicleState> predict(const VehicleState& initial_state,
    double timestep, double delta_t) {
      
      if (!modelLoaded_) {
        throw ModelAccessException("Attempted to use lib_vehicle_model::predict before model was loaded with call to lib_vehicle_model::init()");
      }
      
      // Validate inputs
      if (timestep > delta_t) {
        std::ostringstream msg;
        msg << "Invalid timestep: " << timestep << " is smaller than delta_t : " << delta_t;
        throw std::invalid_argument(msg.str());
      }
      
      constraint_checker_->validateInitialState(initial_state);
      // Pass request to loaded vehicle model
      return vehicle_model_->predict(initial_state, timestep, delta_t);
    }

  std::vector<VehicleState> predict(const VehicleState& initial_state,
    const std::vector<VehicleControlInput>& control_inputs, double timestep) {

      if (!modelLoaded_) {
        throw ModelAccessException("Attempted to use lib_vehicle_model::predict before model was loaded with call to lib_vehicle_model::init()");
      }

      // Validate inputs
      constraint_checker_->validateInitialState(initial_state);
      constraint_checker_->validateControlInputs(initial_state, control_inputs, timestep);

      // Pass request to loaded vehicle model
      return vehicle_model_->predict(initial_state, control_inputs, timestep);
    }
}
