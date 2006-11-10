/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmEndForEachCommand.h,v $
  Language:  C++
  Date:      $Date: 2005/11/16 15:35:29 $
  Version:   $Revision: 1.11 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmEndForEachCommand_h
#define cmEndForEachCommand_h

#include "cmCommand.h"

/** \class cmEndForEachCommand
 * \brief ends an if block
 *
 * cmEndForEachCommand ends an if block
 */
class cmEndForEachCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmEndForEachCommand;
    }

  /**
   * Override cmCommand::InvokeInitialPass to get arguments before
   * expansion.
   */
  virtual bool InvokeInitialPass(std::vector<cmListFileArgument> const&);
  
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&) {return false;}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ENDFOREACH";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Ends a list of commands in a FOREACH block.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ENDFOREACH(expression)\n"
      "See the FOREACH command.";
    }
  
  cmTypeMacro(cmEndForEachCommand, cmCommand);
};


#endif
