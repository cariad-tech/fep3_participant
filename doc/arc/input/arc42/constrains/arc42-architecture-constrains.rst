.. Copyright 2023 CARIAD SE.
   This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed
   with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

.. spelling:word-list::

    CMake
    namespaces
    cpu
    Connext
    mokking
    

.. _label_architecture_constraints:

Architecture Constraints
========================

Platforms
---------

Software Build
~~~~~~~~~~~~~~

The FEP SDK Participant Library as a part of the FEP Co-Simulation Bus is designed to support Windows and Linux platforms. 
Software build requirements are specified by platform specific conan profiles (see: `ADD FEP Co-Simulation Bus <https://devstack.vwgroup.com/confluence/x/CHVMYg>`_ ).

.. todo::
    Clarify with System Architecture team whether following restrictions or detailed information shall be specified on system level (see also chapter Open Source: Software build without conan profiles):
    *   Compiler/Linker/Assembler version (detailed patch version ?)
    *   Compiler/Linker/Assembler options (are there any restrictions, e.g. optimization settings etc.)

Hardware
~~~~~~~~

Platforms
'''''''''

.. todo::
    Clarify with System Architecture team if minimum hardware requirements shall be specified on system level (also relevant for Open Source deliveries) e.g. minimum requirements for heap, stack, ..., cpu?
    `See also SIMHUB CI client hardware requirements <https://devstack.vwgroup.com/confluence/x/5F4CXw>`_ ).


Network infrastructure
''''''''''''''''''''''

The data throughput for a specific network is measured. Details of this measurement can be found `here <https://jfrog.devstack.vwgroup.com/ui/api/v1/download/contentBrowsing/fepdev-generic-releases/fep_sdk/3.2.1/fep/stable/html/advanced_topics/perf_test.html>`_ ).

.. warning::

    The data throughput measurement is outdated!
    
.. todo::
    Is it possible to integrate this measurement in the FEP SDK Participant release process?


Request / Response timings
''''''''''''''''''''''''''

Request / response timings are not measured!



Operating System / Timing
~~~~~~~~~~~~~~~~~~~~~~~~~

There are no timing related requirements for the FEP SDK Participant Library, but the timing behaviour of some features are affected by the Operating System.

.. todo::
    List of components/features with timing constrains (impact of the OS / FEP SDK Participant Library scheduler) 

.. note::
    
    The FEP SDK Participant Library default scheduler is based on OS scheduler but customer can implement an own one which allows no restrictions.

Delivery / Open Source
~~~~~~~~~~~~~~~~~~~~~~

It is required to built the FEP SDK Participant Library with CMake only.

This requires:

*   Specific maintenance of the CMake files to be conan independent
*   Detailed Compiler/Linker/Assembler versions and options (see chapter Software Build)
*   All FEP SDK Participant Library dependencies are open source available

Threading
---------

.. todo::
    Extend public/private API documentation with thread context information/restrictions (e.g. API is called from following thread context ..., are synchronization mechanism for a specific use case necessary, ...)


Security
--------

There are no FEP SDK Participant Library software architecture requirements for security aspects.

.. note::
    
    FEP Simulation Bus: RTI Connext DDS can be used with encryption, authentication and access control enabled but is not tested at all and requires additional licenses. See https://www.rti.com/products/connext-dds-secure



Testing
-------

E.g. design constrains related to the test concept (e.g. class design for mokking, stubbing, etc.)




 