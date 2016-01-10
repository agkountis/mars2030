#ifndef ENEMY_TEMPLATE_H
#define ENEMY_TEMPLATE_H
#include "entity.h"

enum EnemyType{E_NORMAL = 0, NUM_ENEMY_TYPES};

class EnemyTemplate:public Entity
{
private:
    std::string name;
    int hp;
    float speed;
    unsigned int dmg;
    std::string type;
public:
    EnemyTemplate();
    ~EnemyTemplate();
    void set_name(const std::string &name);
    void set_hp(unsigned int hp); //cast it to int
    void set_speed(float sp);
    void set_dmg(unsigned int dmg);
    void set_type(unsigned int type_enum);
    const std::string &get_name() const;
    int get_hp() const;
    float get_speed() const;
    unsigned int get_dmg() const;
    const std::string &get_type() const;
    bool init_tmpl(const char *conf_fname);
};

#endif // ENEMY_TEMPLATE_H
