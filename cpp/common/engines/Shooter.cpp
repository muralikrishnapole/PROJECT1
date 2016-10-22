
#include <SDL2/SDL_opengl.h>
#include "Draw3D.h"
#include "Solids.h"

#include "Shooter.h" // THE HEADER

Projectile3D* Shooter::fireProjectile( Warrior3D * w, double speed, int kind ){
    Projectile3D * p = new Projectile3D();
    p->kind = kind;
    p->id   = shotsCount; shotsCount++;
    p->vel.set_mul ( w->gun_rot, speed );
    p->vel.add     ( w->vel );
    //p->vel.add( { randf(-0.1,0.1), randf(-0.1,0.1), randf(-0.1,0.1) } );
    p->pos.set     ( w->pos );
    p->pos.add_mul ( w->gun_rot, 5.0 );
    projectiles.push_back( p );
    return p;
};

Warrior3D* Shooter::makeWarrior( const Vec3d& pos, const Vec3d& dir, const Vec3d& up, int kind ){
    //int ith = warriors.size();
    //printf( " >>> Setup  ship %i \n", ith );
    Warrior3D* w = new Warrior3D();
    w->kind = kind; w->id = warriorCount; warriorCount++;
    //w->loadFromFile( filename );
    //w->from_mass_points( 2, mass, (Vec2d*)poss );
    w->initOne();

    //printf( " I invI  %f %f \n", ship1->I, ship1->invI );
    //w->setDefaults();
    //w->setAngle( angle );
    w->pos.set ( pos  );
    //w->omega = 0.0;

    w->rotMat.a.set      ( dir     );
    w->rotMat.b.set      ( up      );
    w->rotMat.c.set_cross( dir, up );
    w->qrot.fromMatrix   ( w->rotMat );

    //w->initAllGuns( 6 );
    //printf( "DEBUG 2 \n" );
    //ship->world = this;
    //ship->collisionShape = collisionShape;
    //ship->name = new char[7];
    //sprintf( ship->name, "Ship_%02i", ith );
    //printf( "DEBUG 4 \n" );
    warriors.push_back( w );
    //printf( "DEBUG 5 \n" );
    return w;
}


void Shooter::update_world( ){
	for( int i=0; i<perFrame; i++ ){
//        phi += omega * dt;
//        rot.fromAngle( phi );

        auto itw = warriors.begin();
        while( itw != warriors.end() ) {
            Warrior3D * w = *itw;

            w->clean_temp( );
            //addEnviroForces              ( w->pos, w->vel, w->force,  w->landed );
            //w->landed = collideWithWorld ( w->pos, w->vel, w->surf );
            w->move( dt );

            //w->gun_rot.set_mul_cmplx( rot, w->rot );
            //w->update( dt );

            //printf( " warriro %i pos (%3.3f,%3.3f) vel (%3.3f,%3.3f) force (%3.3f,%3.3f) \n", itw, w->pos.x, w->pos.y, w->vel.x, w->vel.y, w->force.x, w->force.y );

            //printf( " trigger %i until_reaload %f \n ", w->trigger, w->until_reaload );

            ++itw;
        }

        auto it_proj = projectiles.begin();
        while( it_proj != projectiles.end() ) {
            Projectile3D * proj = *it_proj;

            //proj ->update_old_pos(    );
            //proj ->evalForce     (    );
            proj ->move          ( dt );

            Vec3d hRay,normal;
            hRay.set_sub( proj->pos, proj->old_pos );
            double tmax = hRay.normalize();

            bool hitted = false;
            //hitted |= proj->check_hit_ground( );
            //hitted |= proj->check_hit_vector<Frigate2D>( warriors );

            for( o : objects ){
                o->ray( proj->old_pos, hRay, &normal );
                if (hitted) break;
            }
            //if( hitted ){
            if( proj->time > projLifetime ){
                it_proj = projectiles.erase( it_proj );
                delete proj;
            }else{
                ++it_proj;
            }
        }

    }
};


void Shooter::init_world  ( ){ debug_shit=23; };
