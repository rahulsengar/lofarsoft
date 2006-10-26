#
# add the instantiator logic for this kit
#
# takes as arguments:
#   KIT e.g. Common IO
#   CAPS_KIT e.g. COMMON IO
#
MACRO(VTK_USE_INSTANTIATOR KIT CAPS_KIT)
  # Setup vtkInstantiator registration for this library's classes.
  IF (COMMAND VTK_MAKE_INSTANTIATOR2)
    VTK_MAKE_INSTANTIATOR2(vtk${KIT}Instantiator Instantiator_SRCS
                        ${${KIT}_SRCS}
                        EXPORT_MACRO VTK_${CAPS_KIT}_EXPORT
                        HEADER_LOCATION ${VTK_BINARY_DIR})
  ELSE (COMMAND VTK_MAKE_INSTANTIATOR2)
    VTK_MAKE_INSTANTIATOR(vtk${KIT}Instantiator Instantiator_SRCS
                        ${${KIT}_SRCS}
                        EXPORT_MACRO VTK_${CAPS_KIT}_EXPORT
                        HEADER_LOCATION ${VTK_BINARY_DIR})
  ENDIF (COMMAND VTK_MAKE_INSTANTIATOR2)
  ADD_LIBRARY(vtk${KIT}${VTK_VERSION} ${${KIT}_SRCS} ${Instantiator_SRCS})
ENDMACRO(VTK_USE_INSTANTIATOR KIT CAPS_KIT)

#
# generate Tcl wrappers etc
#
# takes arguments:
#   KIT e.g. Common IO
#   DEPENDS e.g. vtkCommonTCL41
#
MACRO(VTK_USE_TCL KIT DEPEND)
  IF (VTK_WRAP_TCL)
    VTK_WRAP_TCL(vtk${KIT}TCL${VTK_VERSION} ${KIT}TCL_SRCS ${${KIT}_SRCS})
    IF (APPLE)
      ADD_LIBRARY(vtk${KIT}TCL${VTK_VERSION} SHARED ${${KIT}TCL_SRCS})
    ELSE (APPLE)
      ADD_LIBRARY(vtk${KIT}TCL${VTK_VERSION} ${${KIT}TCL_SRCS})
    ENDIF (APPLE)
    IF(APPLE OR WIN32)
      TARGET_LINK_LIBRARIES(vtk${KIT}TCL${VTK_VERSION} 
         ${DEPEND} vtk${KIT}${VTK_VERSION} ${TCL_LIBRARY})
    ENDIF(APPLE OR WIN32)
    INSTALL_TARGETS(/lib vtk${KIT}TCL${VTK_VERSION})
  ENDIF (VTK_WRAP_TCL)
ENDMACRO(VTK_USE_TCL)


#
# generate Python wrappers etc
#
# takes arguments:
#   KIT e.g. Common IO
#   DEPENDS e.g. vtkCommonTCL41
#
MACRO(VTK_USE_PYTHON KIT DEPEND)
  IF (VTK_WRAP_PYTHON)
    VTK_WRAP_PYTHON(vtk${KIT}Python${VTK_VERSION} 
      ${KIT}Python_SRCS ${${KIT}_SRCS})
    ADD_LIBRARY(vtk${KIT}Python${VTK_VERSION} MODULE ${${KIT}Python_SRCS})
    IF (NOT APPLE)
      IF(WIN32)
        TARGET_LINK_LIBRARIES (vtk${KIT}Python${VTK_VERSION} ${DEPEND})
      ENDIF(WIN32)
    ENDIF (NOT APPLE)
    IF(WIN32)
      TARGET_LINK_LIBRARIES (vtk${KIT}Python${VTK_VERSION}
                             debug ${PYTHON_DEBUG_LIBRARY}
                             optimized ${PYTHON_LIBRARY})
    ENDIF(WIN32)
    IF(APPLE OR WIN32)
      TARGET_LINK_LIBRARIES(vtk${KIT}Python${VTK_VERSION} 
        vtk${KIT}${VTK_VERSION})
    ENDIF(APPLE OR WIN32)
    INSTALL_TARGETS(/lib vtk${KIT}Python${VTK_VERSION})
  ENDIF (VTK_WRAP_PYTHON)
ENDMACRO(VTK_USE_PYTHON)


#
# generate Python wrappers etc
#
# takes arguments:
#   KIT e.g. Common IO
#   DEPENDS e.g. vtkCommonTCL41
#
MACRO(VTK_USE_JAVA KIT DEPEND)
  IF (VTK_WRAP_JAVA)
    VTK_WRAP_JAVA(vtk${KIT}Java${VTK_VERSION} ${KIT}Java_SRCS ${${KIT}_SRCS})
    ADD_LIBRARY(vtk${KIT}Java${VTK_VERSION} SHARED ${${KIT}Java_SRCS})
    IF(APPLE OR WIN32)
      TARGET_LINK_LIBRARIES (vtk${KIT}Java${VTK_VERSION} 
         ${DEPEND} vtk${KIT}${VTK_VERSION})
    ENDIF(APPLE OR WIN32)
    INSTALL_TARGETS(/lib vtk${KIT}Java${VTK_VERSION})
  ENDIF (VTK_WRAP_JAVA)
ENDMACRO(VTK_USE_JAVA)
