#include "learn.h"

#include "handler.h"
#include "game_classes/classes_spell_slots.h"
#include "game_magic/spells_info.h"
#include "structs/global_objects.h"

class ObjData;

void book_upgrd_fail_message(CharData *ch, ObjData *obj) {
	SendMsgToChar(ch, "Изучив %s от корки до корки вы так и не узнали ничего нового.\r\n",
				 obj->get_PName(3).c_str());
	act("$n с интересом принял$u читать $o3.\r\n"
		"Постепенно $s интерес начал угасать, и $e, плюясь, сунул$g $o3 обратно.",
		false, ch, obj, nullptr, kToRoom);
}

void do_learn(CharData *ch, char *argument, int/* cmd*/, int /*subcmd*/) {
	using PlayerClass::CalcCircleSlotsAmount;

	ObjData *obj;
	int addchance = 10, rcpt = -1;
	im_rskill *rs = nullptr;
	const char *spellname = "";

	const char *stype0[] =
		{
			"секрет",
			"еще несколько секретов"
		};

	const char *stype1[] =
		{
			"заклинание",
			"умение",
			"умение",
			"рецепт",
			"рецепт",
			"способность"
		};

	const char *stype2[] =
		{
			"заклинания",
			"умения",
			"умения",
			"рецепта",
			"способности"
		};

	if (ch->is_npc())
		return;

	// get: blank, spell name, target name
	one_argument(argument, arg);

	if (!*arg) {
		SendMsgToChar("Вы принялись внимательно изучать свои ногти. Да, пора бы и подстричь.\r\n", ch);
		act("$n удивленно уставил$u на свои ногти. Подстриг бы их кто-нибудь $m.",
			false, ch, nullptr, nullptr, kToRoom | kToArenaListen);
		return;
	}

	if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
		SendMsgToChar("А у вас этого нет.\r\n", ch);
		return;
	}

	if (GET_OBJ_TYPE(obj) != EObjType::kBook) {
		act("Вы уставились на $o3, как баран на новые ворота.",
			false, ch, obj, nullptr, kToChar);
		act("$n начал$g внимательно изучать устройство $o1.",
			false, ch, obj, nullptr, kToRoom);
		return;
	}

	if (GET_OBJ_VAL(obj, 0) != EBook::kSpell && GET_OBJ_VAL(obj, 0) != EBook::kSkill &&
		GET_OBJ_VAL(obj, 0) != EBook::kSkillUpgrade && GET_OBJ_VAL(obj, 0) != EBook::kReceipt &&
		GET_OBJ_VAL(obj, 0) != EBook::kFeat) {
		act("НЕВЕРНЫЙ ТИП КНИГИ - сообщите Богам!",
			false, ch, obj, nullptr, kToChar);
		return;
	}

	if (GET_OBJ_VAL(obj, 0) == EBook::kSpell
		&& CalcCircleSlotsAmount(ch, 1) <= 0) {
		SendMsgToChar("Далась вам эта магия! Пошли-бы, водочки выпили...\r\n", ch);
		return;
	}

	if (GET_OBJ_VAL(obj, 2) < 1
		&& GET_OBJ_VAL(obj, 0) != EBook::kSkillUpgrade
		&& GET_OBJ_VAL(obj, 0) != EBook::kSpell
		&& GET_OBJ_VAL(obj, 0) != EBook::kFeat
		&& GET_OBJ_VAL(obj, 0) != EBook::kReceipt) {
		SendMsgToChar("НЕКОРРЕКТНЫЙ УРОВЕНЬ - сообщите Богам!\r\n", ch);
		return;
	}

	if (GET_OBJ_VAL(obj, 0) == EBook::kReceipt) {
		rcpt = im_get_recipe(GET_OBJ_VAL(obj, 1));
	}

	auto skill_id{ESkill::kIncorrect};
	if ((GET_OBJ_VAL(obj, 0) == EBook::kSkill || GET_OBJ_VAL(obj, 0) == EBook::kSkillUpgrade)) {
		skill_id = static_cast<ESkill>(GET_OBJ_VAL(obj, 1));
		if (MUD::Skills().IsInvalid(skill_id)) {
			SendMsgToChar("УМЕНИЕ НЕ ОПРЕДЕЛЕНО - сообщите Богам!\r\n", ch);
			return;
		}
	}
	if (GET_OBJ_VAL(obj, 0) == EBook::kReceipt && rcpt < 0) {
		SendMsgToChar("РЕЦЕПТ НЕ ОПРЕДЕЛЕН - сообщите Богам!\r\n", ch);
		return;
	}
	if (GET_OBJ_VAL(obj, 0) == EBook::kSpell && (GET_OBJ_VAL(obj, 1) < 1
		|| GET_OBJ_VAL(obj, 1) > kSpellCount)) {
		SendMsgToChar("МАГИЯ НЕ ОПРЕДЕЛЕНА - сообщите Богам!\r\n", ch);
		return;
	}
	auto feat_id{EFeat::kIncorrectFeat};
	if (GET_OBJ_VAL(obj, 0) == EBook::kFeat) {
		if ((GET_OBJ_VAL(obj, 1) < EFeat::kFirstFeat || GET_OBJ_VAL(obj, 1) > EFeat::kLastFeat)) {
			SendMsgToChar("СПОСОБНОСТЬ НЕ ОПРЕДЕЛЕНА - сообщите Богам!\r\n", ch);
			return;
		}
		feat_id = static_cast<EFeat>(GET_OBJ_VAL(obj, 1));
	}

	int spellnum = kSpellNoSpell;
	if ((GET_OBJ_VAL(obj, 0) == EBook::kSkill
		&& IsAbleToGetSkill(ch, skill_id, GET_OBJ_VAL(obj, 2)))
		|| GET_OBJ_VAL(obj, 0) == EBook::kSkillUpgrade) {
		spellname = MUD::Skills()[skill_id].GetName();
	} else if (GET_OBJ_VAL(obj, 0) == EBook::kSpell
		&& IsAbleToGetSpell(ch, GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2))) {
		spellnum = GET_OBJ_VAL(obj, 1);
		spellname = spell_info[spellnum].name;
	} else if (GET_OBJ_VAL(obj, 0) == EBook::kReceipt
		&& imrecipes[rcpt].classknow[(int) GET_CLASS(ch)] == KNOW_RECIPE
		&& MAX(GET_OBJ_VAL(obj, 2), imrecipes[rcpt].level) <= GetRealLevel(ch)
		&& imrecipes[rcpt].remort <= GET_REAL_REMORT(ch)) {
		spellnum = rcpt;
		rs = im_get_char_rskill(ch, spellnum);
		spellname = imrecipes[spellnum].name;
		if (imrecipes[spellnum].level == -1 || imrecipes[spellnum].remort == -1) {
			SendMsgToChar("Некорректная запись рецепта для вашего класса - сообщите Богам.\r\n", ch);
			return;
		}
	} else if (GET_OBJ_VAL(obj, 0) == EBook::kFeat && IsAbleToGetFeat(ch, feat_id)) {
		spellname = feat_info[spellnum].name;
	}

	if ((GET_OBJ_VAL(obj, 0) == EBook::kSkill && ch->get_skill(skill_id))
		|| (GET_OBJ_VAL(obj, 0) == EBook::kSpell && GET_SPELL_TYPE(ch, spellnum) & kSpellKnow)
		|| (GET_OBJ_VAL(obj, 0) == EBook::kFeat && HAVE_FEAT(ch, spellnum))
		|| (GET_OBJ_VAL(obj, 0) == EBook::kReceipt && rs)) {
		sprintf(buf, "Вы открыли %s и принялись с интересом\r\n"
					 "изучать. Каким же было разочарование, когда прочитав %s,\r\n"
					 "Вы поняли, что это %s \"%s\".\r\n",
				obj->get_PName(3).c_str(),
				number(0, 1) ? "несколько страниц" :
				number(0, 1) ? "пару строк" : "почти до конца",
				stype1[GET_OBJ_VAL(obj, 0)],
				spellname);
		SendMsgToChar(buf, ch);
		act("$n с интересом принял$u читать $o3.\r\n"
			"Постепенно $s интерес начал угасать, и $e, плюясь, сунул$g $o3 обратно.",
			false, ch, obj, nullptr, kToRoom);
		return;
	}

	if (GET_OBJ_VAL(obj, 0) == EBook::kSkillUpgrade) {
		// апгрейд скилла без учета макс.скилла плеера (до макс в книге)
		if (GET_OBJ_VAL(obj, 3) > 0 && ch->get_trained_skill(skill_id) >= GET_OBJ_VAL(obj, 3)) {
			book_upgrd_fail_message(ch, obj);
			return;
		}

		// апгрейд скилла до макс.скилла плеера (без макса в книге)
		if (GET_OBJ_VAL(obj, 3) <= 0 && ch->get_trained_skill(skill_id) >= CalcSkillRemortCap(ch)) {
			book_upgrd_fail_message(ch, obj);
			return;
		}
	}

	if (!spellnum && MUD::Classes()[ch->get_class()].HasntSkill(skill_id)) {
		const char *where = number(0, 1) ? "вон та" : (number(0, 1) ? "вот эта" : "пятая справа");
		const char *what = number(0, 1) ? "жука" : (number(0, 1) ? "бабочку" : "русалку");
		const char
			*whom = obj->get_sex() == ESex::kFemale ? "нее" : (obj->get_sex() == ESex::kPoly ? "них" : "него");
		sprintf(buf,
				"- \"Какие интересные буковки ! Особенно %s, похожая на %s\".\r\n"
				"Полюбовавшись еще несколько минут на сию красоту, вы с чувством выполненного\r\n"
				"долга закрыли %s. До %s вы еще не доросли.\r\n",
				where, what, obj->get_PName(3).c_str(), whom);
		SendMsgToChar(buf, ch);
		act("$n с интересом осмотрел$g $o3, крякнул$g от досады и положил$g обратно.",
			false, ch, obj, nullptr, kToRoom);
		return;
	}

	addchance = (IS_SORCERER(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForSorcerers)) ||
		(IS_MAGE(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForMages)) ||
		(IS_PALADINE(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForPaladines)) ||
		(IS_THIEF(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForThieves)) ||
		(IS_ASSASINE(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForAssasines)) ||
		(IS_WARRIOR(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForWarriors)) ||
		(IS_RANGER(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForRangers)) ||
		(IS_GUARD(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForGuards)) ||
		(IS_VIGILANT(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForge)) ||
		(IS_MAGUS(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForMaguses)) ||
		(IS_MERCHANT(ch) && ROOM_FLAGGED(ch->in_room, ERoomFlag::kForMerchants)) ? 10 : 0;
	addchance += (GET_OBJ_VAL(obj, 0) == EBook::kSpell) ? 0 : 10;

	if (!obj->has_flag(EObjFlag::KNofail)
		&& number(1, 100) > int_app[POSI(GET_REAL_INT(ch))].spell_aknowlege + addchance) {
		sprintf(buf, "Вы взяли в руки %s и начали изучать. Непослушные\r\n"
					 "буквы никак не хотели выстраиваться в понятные и доступные фразы.\r\n"
					 "Промучившись несколько минут, вы бросили это унылое занятие,\r\n"
					 "с удивлением отметив исчезновение %s.\r\n", obj->get_PName(3).c_str(), obj->get_PName(1).c_str());
		SendMsgToChar(buf, ch);
	} else {
		sprintf(buf, "Вы взяли в руки %s и начали изучать. Постепенно,\r\n"
					 "незнакомые доселе, буквы стали складываться в понятные слова и фразы.\r\n"
					 "Буквально через несколько минут вы узнали %s %s \"%s\".\r\n",
				obj->get_PName(3).c_str(),
				ch->get_skill(skill_id) ? stype0[1] : stype0[0],
				stype2[GET_OBJ_VAL(obj, 0)],
				spellname);
		SendMsgToChar(buf, ch);
		sprintf(buf,
				"LEARN: Игрок %s выучил %s %s \"%s\"",
				GET_NAME(ch),
				(GET_OBJ_VAL(obj, 0) == EBook::kSkillUpgrade) ? stype0[1] : stype0[0],
				stype2[GET_OBJ_VAL(obj, 0)],
				spellname);
		log("%s", buf);
		switch (GET_OBJ_VAL(obj, 0)) {
			case EBook::kSpell: GET_SPELL_TYPE(ch, spellnum) |= kSpellKnow;
				break;

			case EBook::kSkill: ch->set_skill(skill_id, 5);
				break;

			case EBook::kSkillUpgrade: {
				const int left_skill_level = ch->get_trained_skill(skill_id) + GET_OBJ_VAL(obj, 2);
				if (GET_OBJ_VAL(obj, 3) > 0) {
					ch->set_skill(skill_id, std::min(left_skill_level, GET_OBJ_VAL(obj, 3)));
				} else {
					ch->set_skill(skill_id, std::min(left_skill_level, CalcSkillRemortCap(ch)));
				}
			}
				break;

			case EBook::kReceipt: CREATE(rs, 1);
				rs->rid = spellnum;
				rs->link = GET_RSKILL(ch);
				GET_RSKILL(ch) = rs;
				rs->perc = 5;
				break;
			case EBook::kFeat: SET_FEAT(ch, feat_id);
				break;
		}
	}
	ExtractObjFromWorld(obj);
}

