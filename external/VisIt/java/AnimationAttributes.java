// ****************************************************************************
//
// Copyright (c) 2000 - 2006, The Regents of the University of California
// Produced at the Lawrence Livermore National Laboratory
// All rights reserved.
//
// This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or materials provided with the distribution.
//  - Neither the name of the UC/LLNL nor  the names of its contributors may be
//    used to  endorse or  promote products derived from  this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
// CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
// ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************

package llnl.visit;


// ****************************************************************************
// Class: AnimationAttributes
//
// Purpose:
//    This class contains the animation attributes.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   Tue Oct 7 10:45:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

public class AnimationAttributes extends AttributeSubject
{
    // Constants
    public final static int PLAYBACKMODE_LOOPING = 0;
    public final static int PLAYBACKMODE_PLAYONCE = 1;
    public final static int PLAYBACKMODE_SWING = 2;


    public AnimationAttributes()
    {
        super(3);

        pipelineCachingMode = false;
        timeout = 1;
        playbackMode = PLAYBACKMODE_LOOPING;
    }

    public AnimationAttributes(AnimationAttributes obj)
    {
        super(3);

        pipelineCachingMode = obj.pipelineCachingMode;
        timeout = obj.timeout;
        playbackMode = obj.playbackMode;

        SelectAll();
    }

    public boolean equals(AnimationAttributes obj)
    {
        // Create the return value
        return ((pipelineCachingMode == obj.pipelineCachingMode) &&
                (timeout == obj.timeout) &&
                (playbackMode == obj.playbackMode));
    }

    // Property setting methods
    public void SetPipelineCachingMode(boolean pipelineCachingMode_)
    {
        pipelineCachingMode = pipelineCachingMode_;
        Select(0);
    }

    public void SetTimeout(int timeout_)
    {
        timeout = timeout_;
        Select(1);
    }

    public void SetPlaybackMode(int playbackMode_)
    {
        playbackMode = playbackMode_;
        Select(2);
    }

    // Property getting methods
    public boolean GetPipelineCachingMode() { return pipelineCachingMode; }
    public int     GetTimeout() { return timeout; }
    public int     GetPlaybackMode() { return playbackMode; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteBool(pipelineCachingMode);
        if(WriteSelect(1, buf))
            buf.WriteInt(timeout);
        if(WriteSelect(2, buf))
            buf.WriteInt(playbackMode);
    }

    public void ReadAtts(int n, CommunicationBuffer buf)
    {
        for(int i = 0; i < n; ++i)
        {
            int index = (int)buf.ReadByte();
            switch(index)
            {
            case 0:
                SetPipelineCachingMode(buf.ReadBool());
                break;
            case 1:
                SetTimeout(buf.ReadInt());
                break;
            case 2:
                SetPlaybackMode(buf.ReadInt());
                break;
            }
        }
    }


    // Attributes
    private boolean pipelineCachingMode;
    private int     timeout;
    private int     playbackMode;
}

