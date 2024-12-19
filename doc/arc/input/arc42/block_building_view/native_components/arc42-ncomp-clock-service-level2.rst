.. Copyright 2023 CARIAD SE.
   This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed
   with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

.. _label_block_view_native_components_clock_service_lvl2:

Level 2: Native Component: Clock Service
----------------------------------------

Following diagram describes the internal structure of building block *native component clock service* from level 1 as white box.

.. image:: ../../../images/block_building_view/native_components/lvl2_ncomp_clock_service.svg
    :width: 800px

+---------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
| **Name**                  | **Purpose / Responsibility**                                                                                                      | 
+---------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
| Clock Event Sink Registry |  *<Text>*                                                                                                                         |
|                           |  Details can be found :ref:`here<label_block_view_native_components_clock_service_clock_event_sink_registry_lvl3>`                |
+---------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
| Clock Main Event Sink     |  *<Text>*                                                                                                                         |
|                           |  Details can be found :ref:`here<label_block_view_native_components_clock_service_clock_main_event_sink_lvl3>`                    |
+---------------------------+-----------------------------------------------------------------------------------------------------------------------------------+

.. toctree::
    :maxdepth: 3
    
    clock_service/arc42-ncomp-clock-service-level3.rst