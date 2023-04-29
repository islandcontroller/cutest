# cutest

[![License](https://img.shields.io/github/license/islandcontroller/cutest)](LICENSE) [![GitHub](https://shields.io/badge/github-islandcontroller%2Fcutest-black?logo=github)](https://github.com/islandcontroller/cutest) [![Docker Hub](https://shields.io/badge/docker-islandc%2Fcutest-blue?logo=docker)](https://hub.docker.com/r/islandc/cutest) ![Docker Image Version (latest semver)](https://img.shields.io/docker/v/islandc/cutest?sort=semver)

*A lightweight C Unit-Testing Framework for Embedded Applications.*

This framework targets 32-bit, MCU-based embedded application projects written in native C, which already use the *Eclipse Embedded CDT* in their development workflow. It is a powerful tool for test-driven design and other *testing-while-coding* workflows.

> Testing code *right in the process of writing it* will not only reassure developers of their code "*actually working*", but will also decrease the chance of obscure, difficult-to-reproduce bugs finding their way into a finished product undetected.

Have a look at the [**Example Project**](https://github.com/islandcontroller/cutest-example) to get started, and use the provided [**Template Project**](https://github.com/islandcontroller/cutest-template) for setting up new test projects.

### Highlighted Features

* Pure C implementation
* 32-bit MCU target architecture
* Test-cases inlined with application source code
* Super fast test-runs, directly from within Eclipse
    * Error-parser compatible failure reporting
    * Can be configured to run-on-save
* Simple HTML report for documentation

## System requirements

* Docker Engine (running on WSL2, see [notes](#notes) below)
* Eclipse Embedded CDT (Version 11.0 or newer) with Docker Tooling

## Notes

* For running Docker Engine on WSL2, make sure to add the project drive letter as a path mapping to the build configuration, e.g.: `C:|/mnt/c/`. This is already prepared in the template.

* Sometimes Eclipse will not automatically update the header file cache from the Docker container. Usually this is noticed by syntax error markup on all test case and module definitions. To fix this, temporarily de-select "Build inside Docker Image" in project properties. Apply changes, re-enable the container build and apply again.

* Use `#include "<path to appl source>.c"` at the top of your test modules to avoid duplicating application source files into the testing project.

* Define stub interfaces for your instrumented modules to simplify testing of dependent modules. Use `#include <path to stub impl>.inc` to inline the stub source with the test module.

## Acknowledgements

This implementation originates from a heavily customized fork of Asim Jalis' [CuTest](https://cutest.sourceforge.net/), which had proven itself very useful in my development workflow.

## Licensing

Unless stated otherwise, the contents of this project are licensed under the MIT License. The full license text is provided in the [`LICENSE`](LICENSE) file.

    SPDX-License-Identifier: MIT