
#ifndef  GUI_h
#define  GUI_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "Draw.h"
#include "Draw2D.h"
#include "Draw3D.h"

#include <string>

/*

TODO:
 - draw small glyph by simple language ... like:
    goto    100.0 100.0
    line    200.0 250.0
    ngon    6 16.5
*/

//class GUIAbstractPanel;

//extern GUIAbstractPanel* GUI_mouse_panel;
//extern Vec2i GUI_mouse_old_pos;
extern int GUI_fontTex;

//void GUI_globalEventHandler(const SDL_Event* event );



class GUI;


// ==============================
//    class GUITextInput
// ==============================

class GUITextInput{ public:

    int         curPos=0;
	std::string inputText;

    bool     isNumber=true,checkRange=false;
	float    vmin=0.0f, vmax=1.0f;
	double   value=0.0d;
	SDL_Keycode num_op = 0;

	bool     modified=true,entered=false;

	// ==== functions

	void         applyVal( float f );

    virtual void view3D( const Vec3d& pos, int fontTex, float textSize );
    virtual void viewHUD( const Vec2i& pos, int fontTex );
    virtual void onKeyDown( SDL_Event e );
	virtual void onText   ( SDL_Event e );

};

// ==============================
//    class GUIAbstractPanel
// ==============================

class GUIAbstractPanel{ public:
    //int textSz = fontSizeDef;
	int  xmin=256,xmax=128,ymin=0,ymax=0;
	bool visible=true, disabled=false;

	uint32_t bgColor=0xA0A0A0, textColor=0x000000;

	bool     redraw=true;
	int      gllist=0;

	//int      fontTex=0;
    //char*    caption=NULL;
    std::string caption;

	// ==== functions

    void initPanel( const std::string& caption_, int xmin_, int ymin_, int xmax_, int ymax_ );

    GUIAbstractPanel(){};
    GUIAbstractPanel( const std::string& caption, int xmin, int ymin, int xmax, int ymax ){ initPanel(caption, xmin,ymin,xmax,ymax); };

	virtual void moveTo(int x, int y);
	virtual void moveBy(int dx,int dy);

	virtual void              onKeyDown( const SDL_Event& e, GUI& gui );
	virtual GUIAbstractPanel* onMouse( int x, int y, const SDL_Event& event, GUI& gui );
    virtual void              onText( const SDL_Event& e );

    virtual void view  ( );
    virtual void tryRender();



    // inline fnctions

    inline  void draw      ( ){ tryRender(); view(); };
    inline bool check      ( int  x, int  y ){  return (x>xmin)&&(x<xmax)&&(y>ymin)&&(y<ymax); }
	inline void toRelative ( int& x, int& y ){ x-=xmin; y-=ymin; }

};

// ==============================
//       class GUIPanel
// ==============================

class GUIPanel : public GUIAbstractPanel { public:
	bool isSlider=true, isButton=false;

	uint32_t barColor=0x00FF00;

	bool     executed=false;
	int      curPos=0;
	std::string inputText;

	float    vmin=0.0f, vmax=1.0f;
	double   value=0.0d;

	void (*command)(double) = NULL;

    // ==== functions

    GUIPanel(){};
    GUIPanel( const std::string& caption, int xmin, int ymin, int xmax, int ymax, bool isSlider_, bool isButton_ ){ initPanel(caption, xmin,ymin,xmax,ymax); isSlider=isSlider_; isButton=isButton_; };

    virtual void view    ();
	virtual void tryRender();
    virtual void              onKeyDown( const SDL_Event&  e );
    virtual void              onText( const SDL_Event&  e );
    virtual GUIAbstractPanel* onMouse( int x, int y, const SDL_Event& event, GUI& gui );

	// ===== inline functions
	inline double x2val( float  x   ){ return ( x*(vmax-vmin)/(xmax-xmin) )+ vmin; };
	inline float  val2x( double val ){ return (val-vmin)*(xmax-xmin)/(vmax-vmin);  };

};

// ==============================
//     class  ScisorBox
// ==============================

class ScisorBox : public GUIAbstractPanel { public:

    // ==== functions

    void apply();
    void initScisor( const std::string& caption, int xmin_, int ymin_, int xmax_, int ymax_ );

    ScisorBox(){};
    ScisorBox( const std::string& caption, int xmin_, int ymin_, int xmax_, int ymax_ ){ initScisor( caption, xmin_, ymin_, xmax_, ymax_ ); };

    //virtual void draw     ( );
    virtual void tryRender( );
    virtual GUIAbstractPanel* onMouse ( int x, int y, const SDL_Event&  event, GUI& gui );

    //virtual void onKeyDown( SDL_Event e ){};
    //virtual void onText   ( SDL_Event e ){};

};

// ==============================
//     class  MultiPanel
// ==============================

class MultiPanel : public GUIAbstractPanel { public:
    int nsubs;
    GUIPanel ** subs;
    bool opened = true;
    int dy;

    // ==== functions

    void initMulti( const std::string& caption, int xmin_, int ymin_, int xmax_, int ymax_, int nsubs_ );

    MultiPanel(){};
    MultiPanel(const std::string& caption, int xmin, int ymin, int xmax, int ymax, int nsubs){ initMulti( caption, xmin, ymin, xmax, ymax, nsubs ); }

    virtual void open();
    virtual void close();
    void toggleOpen();

    virtual void moveBy(int dx, int dy);

    virtual void view  ( );
    virtual void tryRender( );
    virtual GUIAbstractPanel* onMouse  ( int x, int y, const SDL_Event& event, GUI& gui );

    //virtual void onKeyDown( SDL_Event e, GUI& gui ){};
    //virtual void onText   ( SDL_Event e, GUI& gui ){};

};

// ==============================
//     class  DropDownList
// ==============================

//static const char* exampleDropDownListItems[3] = {"Item1","Item2","Item3"};

class DropDownList : public GUIAbstractPanel { public:
    bool bOpened = true;
    int nSlots=5;
    //int iSlot0=0;

    int iSelected=0;
    int iItem0 = 0;
    //int nItems = 0;
    //char** labels = (char**)exampleDropDownListItems;
    std::vector<std::string> labels;

    //int nsubs;
    //GUIPanel ** subs;

    // ==== functions

    DropDownList* addItem(const std::string& label);
    void initList( const std::string& caption, int xmin_, int ymin_, int xmax_, int nSlots_ );

    DropDownList(){}
    DropDownList( const std::string& caption, int xmin, int ymin, int xmax, int nSlots){ initList(caption,xmin,ymin,xmax,nSlots); }



    virtual void open();
    virtual void close();

    //virtual void view ( );
    virtual void tryRender( );
    virtual GUIAbstractPanel* onMouse  ( int x, int y, const SDL_Event& event, GUI& gui );

    //virtual void onKeyDown( SDL_Event e ){};
    //virtual void onText   ( SDL_Event e ){};

};



// ==============================
//    class GUI
// ==============================

class GUI{ public:
    GUIAbstractPanel* focused = 0;
    GUIAbstractPanel* dragged = 0;
    std::vector<GUIAbstractPanel*> panels;

    GUIAbstractPanel* addPanel( GUIAbstractPanel* panel ){ panels.push_back(panel); return panels.back(); }

    void onEvent( int mouseX, int mouseY, const SDL_Event& event ){
        GUIAbstractPanel* active;
        switch( event.type ){
            case SDL_KEYDOWN:
                //if(focused){ focused->onKeyDown( event ); }else{ txt.onKeyDown(  event ); }; break;
                if(focused){ focused->onKeyDown( event, *this ); }
                break;
            case SDL_TEXTINPUT:
                //if(focused){ focused->onText   ( event ); }else{ txt.onText   ( event );  }; break;
                if(focused){ focused->onText   ( event ); }
                break;
            case SDL_MOUSEBUTTONDOWN:
                active = NULL; focused=NULL;
                for(GUIAbstractPanel* panel: panels){
                    active =  panel->onMouse( mouseX, mouseY, event, *this );
                    focused=active;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT){
                    dragged = 0;
                }
                break;
            case SDL_MOUSEMOTION:
                SDL_MouseMotionEvent* event_ = (SDL_MouseMotionEvent*)&event;
                //if(GUI_mouse_panel) GUI_mouse_panel->moveTo( GUI_mouse_panel->xmin+event->xrel, GUI_mouse_panel->ymin+event->yrel );
                if(dragged){
                    //printf(" GUI_globalEventHandler  SDL_MOUSEMOTION  %i %i \n", event_->xrel, -event_->yrel );
                    dragged->moveBy( event_->xrel, -event_->yrel );
                }
                break;
        };
    };

    void draw(){
        for(GUIAbstractPanel* panel: panels){ panel->draw(); }
    }

    ~GUI(){
        for(GUIAbstractPanel* panel: panels){ delete panel; }
    }

};

#endif
