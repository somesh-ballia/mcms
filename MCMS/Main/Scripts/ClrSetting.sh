#!/bin/bash
# scriptname - ClrSetting

# Text color variables
export TXTUNDERLINE=$(tput sgr 0 1) # Underline
export TXTBOLD=$(tput bold)         # Bold
export TXTRESET=$(tput sgr0)        # Reset

#-------------------------------------------------------
#                                       REGULAR   / BOLD
#-------------------------------------------------------
export COLORBLACK=$(tput setaf 0)    #  black     / grey
export COLORRED=$(tput setaf 1)      #  red       / light red
export COLORGREEN=$(tput setaf 2)    #  green     / light green
export COLORBROWN=$(tput setaf 3)    #  brown     / yellow
export COLORBLUE=$(tput setaf 4)     #  blue      / light blue
export COLORMGNTA=$(tput setaf 5)    #  magenta   / pink
export COLORCYAN=$(tput setaf 6)     #  turquoise / light turquoise
export COLORWHITE=$(tput setaf 7)    #  grey      / white

export BGCOLORBLACK=$(tput setab 0)  #  black     / grey
export BGCOLORRED=$(tput setab 1)    #  red       / light red
export BGCOLORGREEN=$(tput setab 2)  #  green     / light green
export BGCOLORBROWN=$(tput setab 3)  #  brown     / yellow
export BGCOLORBLUE=$(tput setab 4)   #  blue      / light blue
export BGCOLORMGNTA=$(tput setab 5)  #  magenta   / pink
export BGCOLORCYAN=$(tput setab 6)   #  turquoise / light turquoise
export BGCOLORWHITE=$(tput setab 7)  #  grey      / white



