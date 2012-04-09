// THIS FILE CONTAINS SOME DOXYGEN DOCUMENTATION:

/// @file
/// Some Doxygen documentation

// MAIN PAGE

/** @mainpage

The Audio Processing Framework (APF) is a collection of C++ code which was
written in the context multichannel audio applications. However, many modules
have a more generic scope.

Website: http://tu-berlin.de/?id=apf

Blog: http://audio.qu.tu-berlin.de

Development pages: http://dev.qu.tu-berlin.de/projects/apf

Components:

Multithreaded Multichannel Audio Processing Framework: apf::MimoProcessor

C++ wrapper for the JACK Audio Connection Kit (http://jackaudio.org/):
apf::JackClient

IIR second order filter (and cascade of filters): apf::BiQuad and apf::Cascade

Several \ref apf_iterators

Simple matrix class: apf::Matrix

Several different methods to prevent denormals: denormalprevention.h

Some mathematical functions are in the namespace apf::math.

Functions for string manipulation (and conversion) are in the namespace
apf::str.

A parameter map with a few conversion functions: apf::parameter_map

A simple stopwatch: apf::StopWatch

Miscellaneous: misc.h

**/

// GROUPS/MODULES

/**
@defgroup apf_iterators Iterators
TODO: overview of iterators?

@defgroup apf_policies Policies
Policies for apf::MimoProcessor

New policies can be provided in user code!
**/

// EXAMPLES

/**
@example simpleprocessor.h
@example audiofile_simpleprocessor.cpp
@example flext_simpleprocessor.cpp
@example jack_simpleprocessor.cpp
@example mex_simpleprocessor.cpp
@example jack_dynamic_inputs.cpp
@example jack_dynamic_outputs.cpp
@example jack_minimal.cpp
**/

// APF NAMESPACE

/// @namespace apf
/// Audio Processing Framework

// vim:textwidth=80
