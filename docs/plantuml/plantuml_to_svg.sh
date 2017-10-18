#!/bin/bash

# Convert all PlantUML files in this directory to SVG files. The plantuml_jar
# environment variable must be set to the path to PlantUML JAR file.

if [ -z "$plantuml_jar" ]; then
	echo "Usage: plantuml_jar=/path/to/plantuml.jar $0 *.puml" >&2
	exit 1
fi

java -jar "$plantuml_jar" -nometadata -tsvg "$@"

# vim:set noet sts=8 tw=80:
