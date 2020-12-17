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

#include <stdexcept>
#include <math.h>
#include "KinematicsProperty.h"

namespace lib_vehicle_model {
  /**
   * @class KinematicsSolver
   * @brief A namespace which provides a single function called solve which solves the basic kinematic equations which assume constant acceleration using the provided parameters.
   * 
   * The user specifies the KinematicProperty which they wish to calculate as well as the KinematicProperty which they do not wish to include in the calculation.
   */
  namespace KinematicsSolver
  {
    /**
      * @brief Solves for the specified kinematics property based on the provided 3 available properties
      * 
      * The user specifies the KinematicProperty which they wish to calculate as well as the KinematicProperty which they do not wish to include in the calculation.
      * This leaves three available properties to use. These are interpreted according to the following list assuming the output and unavailable properties have been removed
      * 
      * 1. INITIAL_VELOCITY,
      * 2. FINAL_VELOCITY,
      * 3. ACCELERATION,
      * 4. DISTANCE,
      * 5. TIME
      * 
      * For example if solving the kinematic equation d = 0.5 a*t^2 + v_i*t
      * The function would be called as follows double distance = solve(DISTANCE,FINAL_VELOCITY,initial_velocity, acceleration, time);
      * 
      * Only ACCELERATIONS can be negative all other values MUST be positive
      * 
      * @param output_prop The property type would should be solved for
      * @param unavailable_prop The property type which is not provided
      * @param prop1 The first provided property
      * @param prop2 The second provided property
      * @param prop3 The third provided property
      * 
      * @throws std::domain_error if any input other than ACCELERATION is negative
      * @throws std::domain_error if the sign of ACCELERATION does not match the provided change in velocity
      * 
      * @return The value of the output_prop which was solved for
      * 
      */ 
    double solve(const KinematicsProperty& output_prop, const KinematicsProperty& unavailable_prop, const double& prop1, const double& prop2, const double& prop3);
  }
}