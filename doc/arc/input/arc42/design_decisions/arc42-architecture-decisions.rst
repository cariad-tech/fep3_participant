.. Copyright 2023 CARIAD SE.
   This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed
   with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

.. spelling:word-list::

    namespaces
    Clipp
    Connext
    Json
    

.. _label_design_decisions:

Architecture Decisions
======================

Comprehensive decisions
-----------------------

Comprehensive architectural decisions are documented by `Architecture Decision Records (ADRs) <https://devstack.vwgroup.com/confluence/x/LBXRGg>`_.

Compatibility
~~~~~~~~~~~~~

General
"""""""

Compatibility requirements for the FEP SDK Participant Library as part of the FEP SDK are specified in `ADD Compatibility <https://devstack.vwgroup.com/confluence/x/FU8MTw>`_ .

API Compatibility
"""""""""""""""""

The FEP SDK Participant API (backward) compatibility is ensured by the usage of namespaces. Implementation requirements for namespaces and versioning with namespaces are specified in the 
`FEP SDK C++ Programming Guidelines <https://devstack.vwgroup.com/confluence/x/HKo-T>`_ (rules R* and ER*).


ABI Compatibility
"""""""""""""""""

.. todo::
    Describe mechanism how ABI compatibility is ensured.

RPC Compatibility
"""""""""""""""""

.. todo::
    See `here <https://devstack.vwgroup.com/jira/browse/FEPSDK-3169>`_.

Internal decisions
------------------

3'rd party libraries
~~~~~~~~~~~~~~~~~~~~

.. todo::
    Boost, Clipp, RTI Connext DDS, Json RPC, ... : Why are they used (advantages/disadvantages compared to other libraries)?

