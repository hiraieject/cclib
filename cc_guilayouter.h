
#ifndef __GUI_LAYOUTER_H__
#define __GUI_LAYOUTER_H__

class Layouter {
public:
    
    int win_w;
    int win_h;
    int valid_x0;
    int valid_y0;
    int valid_w;
    int valid_h;
    int xmgn;
    int ymgn;
    int btn_w;
    int btn_h;
    
    int x,y,w,h;
    
    Layouter(int _win_w, int _win_h, int _xmgn=20, int _ymgn=10, int _btn_w=150, int _btn_h=40)
    {
        win_w = _win_w;
        win_h = _win_h;
        
        xmgn  = _xmgn;
        ymgn  = _ymgn;
        
        valid_x0 = xmgn*2;
        valid_y0 = ymgn*2;
        valid_w  = win_w - xmgn*4;
        valid_h  = win_h - ymgn*4;
        
        btn_w = _btn_w;
        btn_h = _btn_h;
        
        x = valid_x0;
        y = valid_y0;
        w = 0;
        h = 0;
    }

    void place_btn (int _btn_w=-1, int _btn_h=-1)
    {
        if (w != 0) {
            x += w + xmgn;
        }
        w = _btn_w != -1 ? _btn_w : btn_w;
        h = _btn_h != -1 ? _btn_h : btn_h;
    }

    void place_hr (void)
    {
        if (w != 0) {
            new_line();
        }
        w = valid_w;
        h = ymgn;
    }

    void new_line (void)
    {
        x = valid_x0;
        y += h + ymgn;
        w = 0;
        h = 0;
    }
};

#endif // __GUI_LAYOUTER_H__
