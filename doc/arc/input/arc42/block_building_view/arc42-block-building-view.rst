.. Copyright 2023 CARIAD SE.
   This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed
   with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

.. spelling:word-list::

    Http

.. _label_building_block_view:

Building Block View
===================

The building block view shows the static decomposition of the system into building blocks (modules, components, subsystems, classes,
interfaces, packages, libraries, frameworks, layers, partitions, tiers, functions, macros, operations, data structures, …) as well as their
dependencies (relationships, associations, …).

The building block view is a hierarchical collection of black boxes and white boxes and their descriptions.

**Level 1** is the white box description of the overall system together with black box descriptions of all contained building blocks.

**Level 2** zooms into some building blocks of level 1. Thus it contains the white box description of selected building blocks of level 1,
together with black box descriptions of their internal building blocks.

**Level 3** zooms into selected building blocks of level 2, and so on.

.. _label_block_view_overall_system_lvl1:

Level 1
-------

The decomposition of the overall system starts with a white-box block-building-view diagram at the highest abstraction level with all containing building-blocks as black-boxes. 

.. note::

   For clarity reasons relationships (interfaces) between building blocks are shown on lower abstraction levels. 

.. image:: ../../images/block_building_view/lvl1_participant_overview.svg
    :width: 800px


Contained Building Blocks (black boxes):

+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| **Building Block**  | **Purpose / Responsibility**                                                                | **Details**                                                                      |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| Base                | *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| Core                | *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| Participant         | *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| Plugin              | *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| FEP Http Service Bus| *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| RPC Services        | *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| RTI DDS             | *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| Components          | Components cover a specific functional part of the FEP Participant.                         | Base:                                                                            |
|                     | A FEP Participant provides a set of default components so called **native components**.     |  * <Link>                                                                        |
|                     | The functionality of a FEP Participant can be changed/extended by **foreign components**.   |                                                                                  |
|                     | This building block contains a                                                              | Native Components:                                                               |
|                     |  * Base block which encapsulates shared component functions/interfaces                      |  * :ref:`Clock Service<label_block_view_native_components_clock_service_lvl2>`   |
|                     |  * Native components block:                                                                 |  * *<Link>*                                                                      |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+
| *<black box n>*     | *<Text>*                                                                                    | *<Link>*                                                                         |
+---------------------+---------------------------------------------------------------------------------------------+----------------------------------------------------------------------------------+

.. toctree::
    :maxdepth: 3
    
    native_components/arc42-ncomp-clock-service-level2.rst
