#ifndef ENUM_CLASS_HPP_
#define ENUM_CLASS_HPP_

#include <string>

namespace ClassEnum {
    enum CLASS {
        CLASS_UNKOWN = 0,
        CLASS_WARRIOR,
        CLASS_PRIEST,
        CLASS_MAGE,
        CLASS_ROGUE,
        CLASS_PALADIN,
        CLASS_RANGER,
        CLASS_MERCHANT
    };
    inline CLASS getClassEnum(std::string className) {
        if(className == "warrior") {
            return CLASS_WARRIOR;
        } else if(className == "priest") {
            return CLASS_PRIEST;
        } else if(className == "mage") {
            return CLASS_MAGE;
        } else if(className == "rogue") {
            return CLASS_ROGUE;
        } else if(className == "paladin") {
            return CLASS_PALADIN;
        } else if(className == "ranger") {
            return CLASS_RANGER;
        } else if(className == "merchant") {
            return CLASS_MERCHANT;
        }
        return CLASS_UNKOWN;
    }
    inline std::string getClassString(CLASS classEnum) {
        switch(classEnum) {
            case CLASS_WARRIOR:
                return "warrior";
            case CLASS_PRIEST:
                return "priest";
            case CLASS_MAGE:
                return "mage";
            case CLASS_ROGUE:
                return "rogue";
            case CLASS_PALADIN:
                return "paladin";
            case CLASS_RANGER:
                return "ranger";
            case CLASS_MERCHANT:
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