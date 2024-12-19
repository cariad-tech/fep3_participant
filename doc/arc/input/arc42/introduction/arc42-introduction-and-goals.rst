.. Copyright 2023 CARIAD SE.
   This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed
   with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

.. spelling:word-list::

    runtime
    Hruschka
    Gernot
    Starke
    Graphviz
    

.. _label_introduction_and_goals:

Introduction and Goals
======================

The intention of this document is to develop, document and communicate 
the software architecture of the FEP SDK Participant Library. 

This document goes into detail of following important questions:

* How is the source code of the system organized and structured?
* How are implementation units clustered and how do they relate to each other?
* How do the building blocks behave at runtime and how do they work together?
* What are the fundamental decisions, technologies or concepts that shape the solution, its implementation and operation?

The structure of this document bases on the **arc42** template, tailored to FEP software architecture needs. 

Following arc42 template chapters are **not part of this document**:

+------------------------------+--------------------------------------------------------------------------------------------------+
| **Name**                     | **Reason**                                                                                       | 
+------------------------------+--------------------------------------------------------------------------------------------------+
| *Solution Strategy*          |  *<Text>*                                                                                        |
+------------------------------+--------------------------------------------------------------------------------------------------+
| *Deployment View*            |  *<Text>*                                                                                        |
+------------------------------+--------------------------------------------------------------------------------------------------+
| *Cross-cutting Concepts*     |  see `ADD Cross-cutting Concepts <https://devstack.vwgroup.com/confluence/x/c0gMTw>`_            |
+------------------------------+--------------------------------------------------------------------------------------------------+
| *Quality Requirements*       |  see `ADD Requirements and Vulnerabilities <https://devstack.vwgroup.com/confluence/x/YEgMTw>`_  |
+------------------------------+--------------------------------------------------------------------------------------------------+
| *Risks and Technical Debts*  |  see `ADD Guidance, Restrictions and Risks <https://devstack.vwgroup.com/confluence/x/YUgMTw>`_  |
+------------------------------+--------------------------------------------------------------------------------------------------+

Diagrams of this document uses the unified modelling language (UML) to provide a standard way to visualize the design of this system.
They are modelled by textual descriptions and rendered with PlantUML.   

About arc42
-----------

.. image:: ../../images/arc42-logo.png
    :align: right
    :alt: arc42 logo
    :width: 80px

arc42, the template for documentation of software and system
architecture.

Template Version 8.2 EN. (based upon AsciiDoc version), January 2023

Created, maintained and © by Dr. Peter Hruschka, Dr. Gernot Starke and
contributors. See https://arc42.org.

About PlantUML
--------------

.. image:: ../../images/plantuml_logo.png
   :align: right
   :alt: PlantUML logo
   :width: 80px

`PlantUML <https://plantuml.com/>`_ is an open-source tool to create diagrams from a plain text language.
It uses Graphviz/DOT to compute node positioning for every UML diagrams (except Sequence Diagram).

For modelling Context, Containers and Component diagrams the PlantUML standard library C4 is used. 
See https://c4model.com/.

The C4 model for visualizing software architecture:

.. image:: ../../images/c4-overview.png
   :alt: C4 model
   :width: 800px