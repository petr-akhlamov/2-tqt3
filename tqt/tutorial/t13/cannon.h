/****************************************************************
**
** Definition of CannonField class, TQt tutorial 13
**
****************************************************************/

#ifndef CANNON_H
#define CANNON_H

class TQTimer;


#include <ntqwidget.h>


class CannonField : public TQWidget
{
    TQ_OBJECT
public:
    CannonField( TQWidget *parent=0, const char *name=0 );

    int   angle() const { return ang; }
    int   force() const { return f; }
    bool  gameOver() const { return gameEnded; }
    bool  isShooting() const;
    TQSizePolicy sizePolicy() const;

public slots:
    void  setAngle( int degrees );
    void  setForce( int newton );
    void  shoot();
    void  newTarget();
    void  setGameOver();
    void  restartGame();

private slots:
    void  moveShot();

signals:
    void  hit();
    void  missed();
    void  angleChanged( int );
    void  forceChanged( int );
    void  canShoot( bool );

protected:
    void  paintEvent( TQPaintEvent * );

private:
    void  paintShot( TQPainter * );
    void  paintTarget( TQPainter * );
    void  paintCannon( TQPainter * );
    TQRect cannonRect() const;
    TQRect shotRect() const;
    TQRect targetRect() const;

    int ang;
    int f;

    int timerCount;
    TQTimer * autoShootTimer;
    float shoot_ang;
    float shoot_f;

    TQPoint target;

    bool gameEnded;
};


#endif // CANNON_H
