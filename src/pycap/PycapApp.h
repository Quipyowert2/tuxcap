//--------------------------------------------------
// PycapApp
//
// Pycap application object
//
// Jarrad 'Farbs' Woods
// W.P. van Paassen
// Tony Oakden
//--------------------------------------------------

#ifndef __PYCAPAPP_H__
#define __PYCAPAPP_H__

// must be included first
#define PY_SSIZE_T_CLEAN
#include <Python.h>

// includes
#include "PycapResources.h"
#include "SexyAppBase.h"

#include <string>


// use the Sexy namespace
namespace Sexy
{

// class declarations
class PycapBoard;


// Tutorial Application class
class PycapApp : public SexyAppBase
{
    //------------
    // functions
    //------------

public:
    
    // constructor / destructor
    PycapApp();
    virtual ~PycapApp();

    static PyObject* initModule();

    // Standard SexyApp functions
    virtual void Init(int argc, char*argv[], bool bundled=false);
    virtual void LoadingThreadProc();
    virtual void LoadingThreadCompleted();
    virtual void GotFocus();
    virtual void LostFocus();
    virtual void SwitchScreenMode(bool wantWindowed, bool is3d);

    // Accessor functions
    const PycapResources*   getRes()            { return mResources; }
    const bool              midiInitialized()   { return true; }
    bool                    isPythonBundled()   { return mBundled; }

    // Status reporting functions
    void resLoadFailed()                        { mResFailed = true; }

    // Python helpers
    bool checkPyError();

    //----------
    // members
    //----------

public:

    static PycapApp*            sApp;           // static link to most recent app object (lazy implementation of singleton)
    PyObject*                   pModule;        // python game module
    PyObject*                   pDict;          // dictionary containing python module's namespace. This is a borrowed reference, so it needn't be decref-ed.

private:

    PycapBoard*         mBoard;             // the board currently in use
    PycapResources*     mResources;         // all global images, sounds, fonts etc
    bool                mResFailed;         // whether the application should quit due to global resource loading failure or not
    std::string         mPythonHome;
    std::string         mPythonPath;
    bool                mPythonHomeSet;
    bool                mPythonPathSet;
    bool                mBundled;

    // pycap module functions
    static PyObject* pMarkDirty(PyObject* self, PyObject* args);            // cause a draw call
    static PyObject* pSetColour(PyObject* self, PyObject* args);            // set the current colour
    static PyObject* pSetFont(PyObject* self, PyObject* args);              // set the current font
    static PyObject* pSetColourize(PyObject* self, PyObject* args);         // set the current colour
    static PyObject* pFillRect(PyObject* self, PyObject* args);             // fill a rect using the current colour
    static PyObject* pDrawLine(PyObject* self, PyObject* args);             // draw a line using the start en end position
    static PyObject* pDrawTri(PyObject* self, PyObject* args);              // Draw a Triangle using currently selected color
    static PyObject* pDrawQuad(PyObject* self, PyObject* args);             // Draw a quad using currently selected color
    static PyObject* pDrawQuadTextured(PyObject* self, PyObject* args);     // Fill a quad with a texture
    static PyObject* pDrawImage(PyObject* self, PyObject* args);            // draw an image resource using int position
    static PyObject* pDrawImageF(PyObject* self, PyObject* args);           // draw an image resource using float position
    static PyObject* pDrawImageRot(PyObject* self, PyObject* args);         // draw a rotated image using int position
    static PyObject* pDrawImageRotF(PyObject* self, PyObject* args);        // draw a rotated image using float position
    static PyObject* pDrawImageRotScaled(PyObject* self, PyObject* args);   // draw a rotated and scaled image using float position
    static PyObject* pDrawImageScaled(PyObject* self, PyObject* args);      // draw a scaled image using int position
    static PyObject* pDrawString(PyObject* self, PyObject* args);           // draw a string using the current font
    static PyObject* pShowMouse(PyObject* self, PyObject* args);            // set the mouse cursor on or off
    static PyObject* pDrawmodeNormal(PyObject* self, PyObject* args);       // set the drawing mode to normal
    static PyObject* pDrawmodeAdd(PyObject* self, PyObject* args);          // set the drawing mode to additive
    static PyObject* pPlaySound(PyObject* self, PyObject* args);            // play a sound
    static PyObject* pReadReg(PyObject* self, PyObject* args);              // read from the system registry
    static PyObject* pWriteReg(PyObject* self, PyObject* args);             // write to the system registry
    static PyObject* pPlayTune(PyObject* self, PyObject* args);             // play a midi file
    static PyObject* pStopTune(PyObject* self, PyObject* args);             // stop playing a midi file (or all files if none specified)
    static PyObject* pSetTuneVolume(PyObject* self, PyObject* args);        // change the volume for a song played by the app
    static PyObject* pSetVolume(PyObject* self, PyObject* args);            // change the global volume for all music played by the app
    static PyObject* pSetClipRect(PyObject* self, PyObject* args);          // set the clipping rectangle used by all draw calls
    static PyObject* pClearClipRect(PyObject* self, PyObject* args);        // clear the clipping rectangle used by all draw calls (actually returns it to screen size)
    static PyObject* pSetTranslation(PyObject* self, PyObject* args);       // set the translation applied to subsequent draw calls
    static PyObject* pSetFullscreen(PyObject* self, PyObject* args);        // set whether the app should be in fullscreen or windowed mode
    static PyObject* pGetFullscreen(PyObject* self, PyObject* args);        // get whether the app is in fullscreen or windowed mode
    static PyObject* pAllowAllAccess(PyObject* self, PyObject* args);       // tell the OS that all users can view and modify a file. Required for Vista
    static PyObject* pGetKeyCode(PyObject* self, PyObject* args);           // return a key's keycode
    static PyObject* pGetAppDataFolder(PyObject* self, PyObject* args);     // get the folder that game data should be saved to. Required for Vista
    static PyObject* pGetAppResourceFolder(PyObject* self, PyObject* args); // get the folder where the game resources are stored. Required for GNU/Linux.
    static PyObject* pGetIs3DAccelerated(PyObject* self, PyObject* args);   // returns if the game has 3D acceleration enabled
    static PyObject* pSet3DAccelerated(PyObject* self, PyObject* args);     // set whether application has 3D acceleration enabled or not
    static PyObject* pIsKeyDown(PyObject* self, PyObject* args);            // returns a boolean indicating if the queried key is down
    static PyObject* pGetUserLanguage(PyObject* self, PyObject* args);      // detects the user locale
};


}

// RAII wrapper for global interpreter lock
class GilLocker {
    public:
GilLocker() {
   state = PyGILState_Ensure();
}
~GilLocker() {
    PyGILState_Release(state);
}
PyGILState_STATE state;
};

#endif // __PYCAPAPP_H__
