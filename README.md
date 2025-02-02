# RDMnet

*RDMnet* is an ANSI standard for entertainment technology by [ESTA](http://tsp.esta.org) for
transmission of [RDM](http://www.rdmprotocol.org) over IP networks. RDMnet leverages and extends
the widely-adopted RDM message set and provides a standard method for configuring both IP-based
entertainment equipment and interfaces between IP and DMX512 (commonly referred to as *gateways*).

## About this ETCLabs Project

RDMnet is official, open-source software developed by ETC employees and is designed to interact
with ETC products. For challenges using, integrating, compiling, or modifying this software, we
encourage posting on the [issues page](https://github.com/ETCLabs/RDMnet/issues) of this project.

Before posting an issue or opening a pull request, please read the
[contribution guidelines](./CONTRIBUTING.md).

## Repository Contents

This repository contains a C-language library for core RDMnet communication. It also contains
example applications that demonstrate the roles of Components in RDMnet:

* *rdmnet_controller_example*: A Qt-based GUI application which does basic discovery, display and
  configuration of RDMnet Components.
* *rdmnet_broker_example*: A console application which implements an RDMnet Broker.
* *rdmnet_device_example*: A console application which implements an RDMnet Device.
* *rdmnet_gateway_example*: A console application which implements an RDMnet Gateway,
  affectionately referred to as the "Fakeway". Uses ETC Gadget 2 devices to simulate gateway ports.
* *llrp_manager_example*: A console application which implements a basic LLRP Manager.

The applications are available as a binary package
[on the Releases page](https://github.com/ETCLabs/RDMnet/releases).

The example applications currently support Microsoft Windows, macOS, and Linux, and are built using
CMake. In addition to these platforms, the RDMnet library also supports any plaform targeted by
[EtcPal](https://github.com/ETCLabs/EtcPal), including embedded RTOS platforms such as FreeRTOS.

## Building

For instructions on building the RDMnet library and applications, as well as an
RDMnet overview and in-depth documentation, please see the
[documentation](https://etclabs.github.io/RDMnetDocs).

## Future Plans

This library is maintained by [ETC](http://www.etcconnect.com). This open-source implementation of
an industry standard is intended to encourage adoption of RDMnet throughout the entertainment
industry. The code is licensed under the Apache License 2.0, which allows usage of this library in
commercial and/or closed-source applications.

The current prototypes are mostly compliant with the current standard version, with a few
exceptions:

* The Broker library has no RDM responder functionality and does not implement an LLRP Target
* EPT is not implemented
* The "Request Dynamic UID Assignment" and "Fetch Dynamic UID Assignment List" Broker messages are
  not yet implemented

## Standard Version

The current version of this repository implements ANSI E1.33-2019, published in
August of 2019. You can download the standard document for free from the
[ESTA TSP downloads page](https://tsp.esta.org/tsp/documents/published_docs.php).
