#!/bin/sh

# Setting up Custom IVR slides
export SLIDES_ENV=$(cat $SYSCFG_FILE | grep -A 1 SLIDES_ENV | grep DATA | cut -f2 -d'<' | cut -f2 -d'>')

if [[ "$SLIDES_ENV" == "" ]]; then
	# this default value should be consistent with default defined in SysConfig.cpp
	export SLIDES_ENV=GENERIC
fi

echo "Using IVR Slides environment $SLIDES_ENV"

find Cfg/IVR/Slides/ -type l -exec rm {} \;

if [[ "$SLIDES_ENV" != "GENERIC" ]]; then
	find Cfg/IVR/Custom/Slides/ -type d -name "${SLIDES_ENV}*" -exec ln -sf $MCU_HOME_DIR/mcms/{} Cfg/IVR/Slides/ \;
fi

ls -l Cfg/IVR/Slides
