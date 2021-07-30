#include "../include/Character.h"


void Jack::UseSkill(uint8_t skill, ...) {
	switch (skill) {
	case Skill::PORTAL: {
		mPortal.cast();
		break;
	}
	case Skill::AMBUSH: {
		mAmbush.cast();
		break;
	}
	}
}


void Kaiser::UseSkill(uint8_t skill, ...) {
	va_list ap;
	va_start(ap, skill);

	switch (skill) {
	case Skill::CANON: {
		uint8_t x = va_arg(ap, uint8_t);
		uint8_t y = va_arg(ap, uint8_t);
		// TODO: Gridmap
		//mCanon.cast(x, y, Gridmap);
		break;
	}
	case Skill::ENHANCEMENT: {
		mEnhancement.cast();
		break;
	}
	}
}