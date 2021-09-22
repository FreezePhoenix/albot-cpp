#ifndef ENUM_CLASS_HPP_
#define ENUM_CLASS_HPP_

#include <string>

#define CLASS_UNKOWN -1
#define CLASS_WARRIOR 0
#define CLASS_PRIEST 1
#define CLASS_MAGE 2
#define CLASS_ROGUE 3
#define CLASS_PALADIN 4
#define CLASS_RANGER 5
#define CLASS_MERCHANT 6

namespace ClassEnum {
    enum CLASS {
        UNKOWN      = CLASS_UNKOWN,
        WARRIOR     = CLASS_WARRIOR,
        PRIEST      = CLASS_PRIEST,
        MAGE        = CLASS_MAGE,
        ROGUE       = CLASS_ROGUE,
        PALADIN     = CLASS_PALADIN,
        RANGER      = CLASS_RANGER,
        MERCHANT    = CLASS_MERCHANT
    };
    inline CLASS getClassEnum(std::string className) {
        if(className == "warrior") {
            return WARRIOR;
        } else if(className == "priest") {
            return PRIEST;
        } else if(className == "mage") {
            return MAGE;
        } else if(className == "rogue") {
            return ROGUE;
        } else if(className == "paladin") {
            return PALADIN;
        } else if(className == "ranger") {
            return RANGER;
        } else if(className == "merchant") {
            return MERCHANT;
        }
        return UNKOWN;
    }
    inline std::string getClassString(CLASS classEnum) {
        switch(classEnum) {
            case WARRIOR:
                return "warrior";
            case PRIEST:
                return "priest";
            case MAGE:
                return "mage";
            case ROGUE:
                return "rogue";
            case PALADIN:
                return "paladin";
            case RANGER:
                return "ranger";
            case MERCHANT:
                return "merchant";
            default:
                return "unkown";
        }
    }
    inline std::string getClassStringInt(int classEnum) {
        return ClassEnum::getClassString(static_cast<CLASS>(classEnum));
    }
}

#endif /* ENUM_CLASS_HPP_ */