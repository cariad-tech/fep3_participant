.. Copyright 2023 CARIAD SE.
   This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed
   with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

.. spelling:word-list::

    Runtime
    
    
.. _label_runtime_view:

Runtime View
============

.. _label_runtime_view_scenario_1:

<Runtime Scenario 1: FEP Element Start-Up>
------------------------------------------

Following scenario shows the start-up behaviour of a simplified FEP Element. 

FEP Element: Create and execute 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+
| .. image:: ../../images/runtime_view/scenario_startup/fep_element_create_execute.svg | .. image:: ../../images/runtime_view/scenario_startup/fep_element_state_unloaded.svg    |
|     :width: 800px                                                                    |     :width: 400px                                                                       | 
+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+


-  *<insert description of the notable aspects of the interactions
   between the building block instances depicted in this diagram.>*
   
Runtime Scenario 1.1: Create/Register Components: Clock/Scheduler

Register Components: Clock/Scheduler

.. image:: ../../images/runtime_view/scenario_startup/clock_scheduler/components_register_clock_scheduler.svg
    :width: 800px

    
Create Components: Clock/Scheduler

.. image:: ../../images/runtime_view/scenario_startup/clock_scheduler/components_create_clock_scheduler.svg
    :width: 800px


FEP Element: Load
~~~~~~~~~~~~~~~~~
 
+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+
| .. image:: ../../images/runtime_view/scenario_startup/fep_element_load.svg           | .. image:: ../../images/runtime_view/scenario_startup/fep_element_state_loaded.svg      |
|     :width: 800px                                                                    |     :width: 400px                                                                       | 
+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+
 

FEP Element: Initialize
~~~~~~~~~~~~~~~~~~~~~~~ 

+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+
| .. image:: ../../images/runtime_view/scenario_startup/fep_element_initialize.svg     | .. image:: ../../images/runtime_view/scenario_startup/fep_element_state_initialized.svg |
|     :width: 800px                                                                    |     :width: 400px                                                                       | 
+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+

Runtime Scenario 1.2: Initialize Components: Clock/Scheduler

Initialize Components: Clock/Scheduler - timing master (discrete)

.. image:: ../../images/runtime_view/scenario_startup/clock_scheduler/components_init_clock_scheduler_master_discrete.svg
    :width: 400px

Tense Components: Clock/Scheduler - timing master (discrete)

.. image:: ../../images/runtime_view/scenario_startup/clock_scheduler/components_tense_clock_scheduler_master_discrete.svg
    :width: 800px
    
Initialize Components: Clock/Scheduler - timing slave (discrete)

.. image:: ../../images/runtime_view/scenario_startup/clock_scheduler/components_init_clock_scheduler_slave_discrete.svg
    :width: 800px
    
Tense Components: Clock/Scheduler - timing slave (discrete)

<TBD>

FEP Element: Running
~~~~~~~~~~~~~~~~~~~~ 

+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+
| .. image:: ../../images/runtime_view/scenario_startup/fep_element_start.svg          | .. image:: ../../images/runtime_view/scenario_startup/fep_element_state_running.svg     |
|     :width: 800px                                                                    |     :width: 400px                                                                       | 
+--------------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------+

Start Components: Clock/Scheduler - timing master (discrete)

.. image:: ../../images/runtime_view/scenario_startup/clock_scheduler/components_start_clock_scheduler_master_discrete.svg
    :width: 800px


.. _label_runtime_view_scenario_2:

<Runtime Scenario 2>
--------------------


.. _label_runtime_view_scenario_n:

<Runtime Scenario n>
--------------------