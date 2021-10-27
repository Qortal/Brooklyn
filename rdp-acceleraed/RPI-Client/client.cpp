/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
 * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcm_host.h"

extern "C" {
    #include "ilclient.h"
}

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include "mouse.h"

using boost::asio::ip::tcp;
using namespace std;

enum {
    max_length = 1024
};

static int start_decode_video(char* host, char* port) {
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
    COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL, *clock = NULL;
    COMPONENT_T * list[5];
    TUNNEL_T tunnel[4];
    ILCLIENT_T *client;
    int status = 0;
    unsigned int data_len = 0;

    memset(list, 0, sizeof (list));
    memset(tunnel, 0, sizeof (tunnel));


    if ((client = ilclient_init()) == NULL) {
        return -3;
    }

    if (OMX_Init() != OMX_ErrorNone) {
        ilclient_destroy(client);
        return -4;
    }

    // create video_decode
    if (ilclient_create_component(client, &video_decode, "video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
        status = -14;
    list[0] = video_decode;

    // create video_render
    if (status == 0 && ilclient_create_component(client, &video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    list[1] = video_render;

    // create clock
    if (status == 0 && ilclient_create_component(client, &clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    list[2] = clock;

    memset(&cstate, 0, sizeof (cstate));
    cstate.nSize = sizeof (cstate);
    cstate.nVersion.nVersion = OMX_VERSION;
    cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
    cstate.nWaitMask = 1;
    if (clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
        status = -13;

    // create video_scheduler
    if (status == 0 && ilclient_create_component(client, &video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    list[3] = video_scheduler;

    set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
    set_tunnel(tunnel + 1, video_scheduler, 11, video_render, 90);
    set_tunnel(tunnel + 2, clock, 80, video_scheduler, 12);

    // setup clock tunnel first
    if (status == 0 && ilclient_setup_tunnel(tunnel + 2, 0, 0) != 0)
        status = -15;
    else
        ilclient_change_component_state(clock, OMX_StateExecuting);

    if (status == 0)
        ilclient_change_component_state(video_decode, OMX_StateIdle);

    memset(&format, 0, sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof (OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 130;
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;

    if (status == 0 &&
        OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
        ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0) {
        OMX_BUFFERHEADERTYPE *buf;
        int port_settings_changed = 0;
        int first_packet = 1;

        ilclient_change_component_state(video_decode, OMX_StateExecuting);


        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), host, port);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        boost::asio::connect(s, iterator);
        s.set_option(tcp::no_delay(true));

        boost::asio::write(s, boost::asio::buffer("a", 2));

        while ((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL) {
            // feed data and wait until we get port settings changed
            unsigned char *dest = buf->pBuffer;

            int bufferSize = std::min((int)buf->nAllocLen, 10 * 1024);
            data_len += boost::asio::read(s, boost::asio::buffer(dest, bufferSize));

            if (port_settings_changed == 0 &&
                ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
                (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0))) {
                port_settings_changed = 1;

                if (ilclient_setup_tunnel(tunnel, 0, 0) != 0) {
                    status = -7;
                    break;
                }

                ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

                // now setup tunnel to video_render
                if (ilclient_setup_tunnel(tunnel + 1, 0, 1000) != 0) {
                    status = -12;
                    break;
                }

                ilclient_change_component_state(video_render, OMX_StateExecuting);
            }
            if (!data_len)
                break;

            buf->nFilledLen = data_len;
            data_len = 0;

            buf->nOffset = 0;
            if (first_packet) {
                buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
                first_packet = 0;
            } else
                buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

            if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone) {
                status = -6;
                break;
            }
        }

        buf->nFilledLen = 0;
        buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

        if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
            status = -20;

        // wait for EOS from render
        ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
            ILCLIENT_BUFFER_FLAG_EOS, 10000);

        // need to flush the renderer to allow video_decode to disable its input port
        ilclient_flush_tunnels(tunnel, 0);

        ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
    }

    ilclient_disable_tunnel(tunnel);
    ilclient_disable_tunnel(tunnel + 1);
    ilclient_disable_tunnel(tunnel + 2);
    ilclient_teardown_tunnels(tunnel);

    ilclient_state_transition(list, OMX_StateIdle);
    ilclient_state_transition(list, OMX_StateLoaded);

    ilclient_cleanup_components(list);

    OMX_Deinit();

    ilclient_destroy(client);
    return status;
}

char *key_name[] = {
    "first",
    "second (or middle)",
    "third"
};

struct SendStruct {
    int type;
    int x;
    int y;
    int button;
    int keycode;
};

static void FillRect( void *image, int pitch, int x, int y, int w, int h, int val )
{
    int         row;
    int         col;

    uint32_t *line = (uint32_t *)image + y * (pitch>>2) + x;

    for ( row = 0; row < h; row++ )
    {
        for ( col = 0; col < w; col++ )
        {
            line[col] = val;
        }
        line += (pitch>>2);
    }
}

void mouseKeyboardThread(char* host, char* port)
{
    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), host, port);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    tcp::socket s(io_service);
    boost::asio::connect(s, iterator);
	s.set_option(tcp::no_delay(true));

    boost::asio::write(s, boost::asio::buffer("b", 2));
        
    Display *display;
    XEvent xevent;
    Window window;

    if( (display = XOpenDisplay(NULL)) == NULL )
        return;


    window = DefaultRootWindow(display);
    XAllowEvents(display, AsyncBoth, CurrentTime);

    XGrabPointer(display, 
                 window,
                 1, 
                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask , 
                 GrabModeAsync,
                 GrabModeAsync, 
                 None,
                 None,
                 CurrentTime);

    XGrabKeyboard(display, window, false, GrabModeAsync, GrabModeAsync, CurrentTime);

    Mouse mouse;
    
    while(1) {
        XNextEvent(display, &xevent);
        int mykey;
        SendStruct send;
        SendStruct* sendPtr = &send;
        switch (xevent.type) {
            case MotionNotify:
                send.type = 0;
                send.x = xevent.xmotion.x_root;
                send.y = xevent.xmotion.y_root;
                
                mouse.move(send.x, send.y);
                
                //printf("Mouse move      : [%d, %d]\n", xevent.xmotion.x_root, xevent.xmotion.y_root);
                break;
            case ButtonPress:
                send.type = 1;
                send.button = xevent.xbutton.button;
                //printf("Button pressed  : %s\n", key_name[xevent.xbutton.button - 1]);
                break;
            case ButtonRelease:
                send.type = 2;
                send.button = xevent.xbutton.button;
                //printf("Button released : %s\n", key_name[xevent.xbutton.button - 1]);
                break;
            case KeyPress:
                mykey = XKeycodeToKeysym(display, xevent.xkey.keycode, 0);
                //printf("KeyPress : %s, %d\n", XKeysymToString(mykey), mykey);
                
                send.type = 3;
                send.keycode = mykey;
                
                if (xevent.xkey.keycode == 27 || xevent.xkey.keycode == 9) {
                    return;
                }
                break;
            case KeyRelease:
                mykey = XKeycodeToKeysym(display, xevent.xkey.keycode, 0);
                
                send.type = 4;
                send.keycode = mykey;
                
                //printf("KeyRelease : %s, %d\n", XKeysymToString(mykey), mykey);
                break;
        }
        boost::asio::write(s, boost::asio::buffer(sendPtr, sizeof(SendStruct)));
    }
    
    mouse.close();
}

int main(int argc, char **argv) {
    std::cout << "Version 0.9" << endl;
    
    if (argc != 3)
    {
      std::cerr << "Usage: ./client <host> <port>\n";
      return 1;
    }
    
    char* host = argv[1];
    char* port = argv[2];
    
    bcm_host_init();
    
    boost::thread t(&mouseKeyboardThread, host, port);
    start_decode_video(host, port);
    t.join();
}