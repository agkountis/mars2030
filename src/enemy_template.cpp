#include "enemy_template.h"
#include "cfg.h"

EnemyTemplate::EnemyTemplate()
{

}

EnemyTemplate::~EnemyTemplate()
{

}

void EnemyTemplate::set_name(const std::string &name)
{
    this->name = name;
}

void EnemyTemplate::set_hp(unsigned int hp)
{
    this->hp = hp;
}

void EnemyTemplate::set_speed(float sp)
{
    speed = sp;
}

void EnemyTemplate::set_dmg(unsigned int dmg)
{
    this->dmg = dmg;
}

void EnemyTemplate::set_type(unsigned int type_enum)
{
    type = type_enum;
}

const std::string& EnemyTemplate::get_name() const
{
    return name;
}

int EnemyTemplate::get_hp() const
{
    return hp;
}

float EnemyTemplate::get_speed() const
{
    return speed;
}

unsigned int EnemyTemplate::get_dmg() const
{
    return dmg;
}

const std::string &EnemyTemplate::get_type() const
{
    return type;
}

bool EnemyTemplate::init_tmpl(const char *conf_fname)
{
    ConfigFile cfg(conf_fname);

    if(!cfg.is_open())
    {
        std::cout<<"Could not parse"<<conf_fname<<"! Aborting!"<<std::endl;
        return false;
    }

    name = cfg.get_str("attributes.name");
    hp = cfg.get_int("attributes.hp",0);
    speed = cfg.get_num("attributes.speed",1);
    dmg = cfg.get_int("attributes.dmg",0);
    type = (std::string)cfg.get_str("attributes.type","normal");

    init_b_obj();

    return true;
}

