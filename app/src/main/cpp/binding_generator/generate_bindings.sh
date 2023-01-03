#!/bin/sh
perl generate_imgui_bindings.pl < ../includes/imgui/imgui.h > ../bindings/inline.h
