#pragma once
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

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <lib_vehicle_model/ODESolver.h>
#include <lib_vehicle_model/VehicleState.h>
#include <lib_vehicle_model/VehicleMotionModel.h>
#include <lib_vehicle_model/VehicleControlInput.h>
#include <lib_vehicle_model/ParameterServer.h>
 
/**
 * @class PassengerCarDynamicModel
 * @brief Class which implements a dynamic vehicle model for front wheel drive passenger cars
 * 
 * The details of the math in this class can be found in the PassengerCarDynamicBicycleModel design document
 * 
 * NOTE: This class does not support trailers at this time. The trailer angle will remain unchanged during integration
 * 
 */
class PassengerCarDynamicModel: public lib_vehicle_model::VehicleMotionModel
{
  private:

    const size_t ODE_STATE_SIZE = 9; // Number of elements of VehicleState that are accounted for in this model
    const size_t FULL_STATE_SIZE = 12; // Total number of elements in a CARMA VehicleState 

    // Handles to callback functions
    lib_vehicle_model::ODESolver::ODEFunction<lib_vehicle_model::VehicleControlInput, double> ode_func_;
    lib_vehicle_model::ODESolver::PostStepFunction<lib_vehicle_model::VehicleControlInput, double> post_step_func_;
    
    // Parameter server used to load vehicle parameters
    std::shared_ptr<lib_vehicle_model::ParameterServer> param_server_;
    
    // Parameters
    double l_f_; // The distance from the center of mass to the front wheels axis along the vehicle center line in m.
    double l_r_; // The distance from the center of mass to the rear wheels axis along the vehicle center line in m. 
    double R_ef_; // The vertical distance from the front axle to the ground when the vehicle is loaded. 
    double R_er_; // The vertical distance from the rear axle to the ground when the vehicle is loaded. 
    double C_sx_; // The longitudinal tire stiffness in N/unit slip. 
    double C_ay_; // The tire cornering stiffness in N/rad. 
    double I_z_; // The moment of inertia of the vehicle about its center of mass in kgm^2
    double m_; // The vehicle mass in kg.

    /*
     * @brief Function describing the ODE system which defines the vehicle equations of motion
     * 
     * This function matches the ODESolver::ODEFunction definition
     */ 
    void DynamicCarODE(const lib_vehicle_model::ODESolver::State& state,
      const lib_vehicle_model::VehicleControlInput& control,
      double& prev_time,
      lib_vehicle_model::ODESolver::StateDot& state_dot,
      double t
    ) const;

    /*
     * @brief Function describing the necessary actions after each ODE integration step
     * 
     * This function matches the ODESolver::PostStepFunction definition.
     * This function is used to populate the state vector elements not used in the ODE
     */ 
    void ODEPostStep(const lib_vehicle_model::ODESolver::State& current,
      const lib_vehicle_model::VehicleControlInput& control,
      double& prev_time,
      double t,
      const lib_vehicle_model::ODESolver::State& initial_state,
      lib_vehicle_model::ODESolver::State& output
    ) const;

    /**
     * @brief Helper function defines the transfer function which converts new velocity commands into front wheel rotation rate rates of change
     * 
     * @param w_f The current front wheel speed
     * @param w_r The current rear wheel speed
     * @param V_c The new velocity command in m/s
     */ 
    double funcW_f(const double w_f, const double w_r, const double V_c) const;

    /**
     * @brief Helper function defines the transfer function which converts new velocity commands into rear wheel rotation rate rates of change
     * 
     * @param w_f The current front wheel speed
     * @param w_r The current rear wheel speed
     * @param V_c The new velocity command in m/s
     */ 
    double funcW_r(const double w_f, const double w_r, const double V_c) const;

    /**
     * @brief Helper function defines the transfer function which converts new steering commands into steering rates of change
     * 
     * @param d_f The current steering angle in rad
     * @param d_fc The new steering command in rad
     */ 
    double funcD_f(const double d_f, const double d_fc) const;
    

  public:

    /**
     * @brief Constructor 
     * 
     */ 
    PassengerCarDynamicModel();

    /**
     * @brief Destructor as required by interface
     * 
     */ 
    ~PassengerCarDynamicModel();

    //
    // Overriden interface functions
    //

    /**
     * @throws std::invalid_argment if not all required parameters could be read
     */ 
    void setParameterServer(std::shared_ptr<lib_vehicle_model::ParameterServer> parameter_server) override;

    std::vector<lib_vehicle_model::VehicleState> predict(const lib_vehicle_model::VehicleState& initial_state,
      double timestep, double delta_t) override; 

    std::vector<lib_vehicle_model::VehicleState> predict(const lib_vehicle_model::VehicleState& initial_state,
      const std::vector<lib_vehicle_model::VehicleControlInput>& control_inputs, double timestep) override;
};
