#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "log.hpp"
#include "remote.hpp"
#include "hack.hpp"

using namespace std;

bool shouldGlow = true;


int main() {
    if (getuid() != 0) {
        cout << "You should run this as root." << endl;
        return 0;
    }

    cout << "s0beit linux hack version 1.3" << endl;

    log::init();
    log::put("Hack loaded...");
/*
	Display* dpy = XOpenDisplay(0);
	Window root = DefaultRootWindow(dpy);
	XEvent ev;

	int keycode = XKeysymToKeycode(dpy, XK_X);
	unsigned int modifiers = 0;

	XGrabKey(dpy, keycode, modifiers, root, false,
				GrabModeAsync, GrabModeAsync);

	XSelectInput(dpy, root, modifiers);
	*/
    remote::Handle csgo;

    while (true) {
        if (remote::FindProcessByName("csgo_linux64", &csgo)) {
            break;
        }

        usleep(500);
    }

    cout << "CSGO Process Located [" << csgo.GetPath() << "][" << csgo.GetPid() << "]" << endl << endl;

    remote::MapModuleMemoryRegion client;

    client.start = 0;

    while (client.start == 0) {
        if (!csgo.IsRunning()) {
            cout << "Exited game before client could be located, terminating" << endl;
            return 0;
        }

        csgo.ParseMaps();

        for (auto region : csgo.regions) {
            if (region.filename.compare("client_client.so") == 0 && region.executable) {
                cout << "client_client.so: [" << std::hex << region.start << "][" << std::hex << region.end << "][" <<
                region.pathname << "]" << endl;
                client = region;
                break;
            }
        }

        usleep(500);
    }

    cout << "GlowObject Size: " << std::hex << sizeof(hack::GlowObjectDefinition_t) << endl;

    cout << "Found client_client.so [" << std::hex << client.start << "]" << endl;
    client.client_start = client.start;


// Old sig x86 "\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6"
//                                             "x????xxxxx"
//Old Sig Pre 11/10/15
//\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6\x34
//x????xxxxxx
//New Sig as of 11/10/15
//\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6\x38
//x????xxxxxx
//11/10/15 Sig reduction, we don't need the size
//\xE8\x00\x00\x00\x00\x8B\x78\x14\x6B\xD6
//x????xxxxx

 void* foundGlowPointerCall = client.find(csgo,
                                             "\xE8\x00\x00\x00\x00\x48\x8b\x10\x48\xc1\xe3\x06\x44",
                                             "x????xxxxxxxx");

    cout << "Glow Pointer Call Reference: " << std::hex << foundGlowPointerCall <<
    " | Offset: " << (unsigned long) foundGlowPointerCall - client.start << endl;

    unsigned long call = csgo.GetCallAddress(foundGlowPointerCall);

    cout << "Glow function address: " << std::hex << call << endl;
    cout << "Glow function address offset: " << std::hex << call - client.start << endl;


    csgo.m_addressOfGlowPointer = csgo.GetCallAddress((void*)(call+0xF));
    cout << "Glow Array pointer " << std::hex << csgo.m_addressOfGlowPointer << endl << endl;


  // long ptrLocalPlayer = (client->client_start + 0x5A9B1A0); 27/06/16
    unsigned long foundLocalPlayerLea = (long)client.find(csgo,
                                             "\x48\x89\xe5\x74\x0e\x48\x8d\x05\x00\x00\x00\x00", //27/06/16
                                             "xxxxxxxx????");

    csgo.m_addressOfLocalPlayer = csgo.GetCallAddress((void*)(foundLocalPlayerLea+0x7));

    unsigned long foundAttackMov = (long)client.find(csgo,
                                             "\x44\x89\xe8\x83\xe0\x01\xf7\xd8\x83\xe8\x03\x45\x84\xe4\x74\x00\x21\xd0", //10/07/16
                                             "xxxxxxxxxxxxxxx?xx");
    csgo.m_addressOfForceAttack = csgo.GetCallAddress((void*)(foundAttackMov+19));


    while (csgo.IsRunning()) {
/*
		while (XPending(dpy) > 0) {
			XNextEvent(dpy, &ev);
			switch (ev.type) {
				case KeyPress:
					
					XUngrabKey(dpy, keycode, modifiers, root);
					//shouldGlow = !shouldGlow;
					csgo.m_shouldTrigger = !csgo.m_shouldTrigger ;
					cout << "Toggling trigger... " << csgo.m_shouldTrigger << endl;
					break;
				default:
					break;
			}

			XGrabKey(dpy, keycode, modifiers, root, false,
						GrabModeAsync, GrabModeAsync);
			XSelectInput(dpy, root, KeyPressMask);

		}
*/
		if (shouldGlow)
	        	hack::Glow(&csgo, &client);
	usleep(10);
	}
//    cout << "Game ended." << endl;

    return 0;
}
