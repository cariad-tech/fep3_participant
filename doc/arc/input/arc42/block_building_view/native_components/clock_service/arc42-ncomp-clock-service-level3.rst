.. Copyright 2023 CARIAD SE.
   This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed
   with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
.. _label_block_view_native_components_clock_service_lvl3:

Level 3: Clock Service
----------------------

.. _label_block_view_native_components_clock_service_clock_event_sink_registry_lvl3:

*Native Component - Clock Service: Clock Event Sink Registry*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Specifies the internal structure of *Native Component - Clock Service: Clock Event Sink Registry*.

Following diagram describes the internal structure of building block *native component clock service: clock event sink registry* from level 2 as white box.

.. image:: ../../../../images/block_building_view/native_components/clock_service/lvl3_ncomp_clock_event_sink_registry.svg
    :width: 800px

+-----------------------+---------------------------------------------------------------------------------------------+
| **Interface**         | **Description**                                                                             | 
+-----------------------+---------------------------------------------------------------------------------------------+
| *<Interface>*         |  *<Text>*                                                                                   |
+-----------------------+---------------------------------------------------------------------------------------------+

.. _label_block_view_native_components_clock_service_clock_main_event_sink_lvl3:

*Native Component - Clock Service: Clock Main Event Sink*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Specifies the internal structure of *Native Component - Clock Service: Clock Main Event Sink*.

.. image:: ../../../../images/block_building_view/native_components/clock_service/lvl3_ncomp_clock_main_event_sink.svg
    :width: 800px

+------------------------------------------------------+---------------------------------------------------------------------------------------------+
| **Interface**                                        | **Description**                                                                             | 
+------------------------------------------------------+---------------------------------------------------------------------------------------------+
| :cpp:class:`fep3::arya::ILogger`                     |                                                                                             |
+------------------------------------------------------+---------------------------------------------------------------------------------------------+
| :cpp:class:`fep3::rpc::arya::IRPCClockSyncMasterDef` |                                                                                             | 
+------------------------------------------------------+---------------------------------------------------------------------------------------------+
| *<RPCClockSyncClient>*                               | .. warning::                                                                                |
|                                                      |    Internal interfaces currently not documented!!!                                          |
+------------------------------------------------------+---------------------------------------------------------------------------------------------+
| *<Interface>*                                        |  *<Text>*                                                                                   |
+------------------------------------------------------+---------------------------------------------------------------------------------------------+


.. _label_block_view_native_components_clock_service_x:

White Box <_building block x>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*<white box template>*

.. _label_block_view_native_components_clock_service_y:

White Box <_building block y>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*<white box template>*