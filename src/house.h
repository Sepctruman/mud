/* ****************************************************************************
* File: house.h                                                Part of Bylins *
* Usage: Handling of clan system                                              *
* (c) 2005 Krodo                                                              *
******************************************************************************/

#ifndef _HOUSE_H_
#define _HOUSE_H_

#include <vector>
#include <map>
#include <bitset>
#include <string>
#include <boost/shared_ptr.hpp>
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "interpreter.h"
#include "house_exp.hpp"
#include "remember.hpp"

namespace ClanSystem
{

enum Privileges: unsigned
{
	MAY_CLAN_INFO = 0,
	MAY_CLAN_ADD,
	MAY_CLAN_REMOVE,
	MAY_CLAN_PRIVILEGES,
	MAY_CLAN_CHANNEL,
	MAY_CLAN_POLITICS,
	MAY_CLAN_NEWS,
	MAY_CLAN_PKLIST,
	MAY_CLAN_CHEST_PUT,
	MAY_CLAN_CHEST_TAKE,
	MAY_CLAN_BANK,
	MAY_CLAN_EXIT,
	MAY_CLAN_MOD,
	MAY_CLAN_TAX,
	/// ����� ����������
	CLAN_PRIVILEGES_NUM
};

const unsigned MAX_GOLD_TAX_PCT = 25;
const int MIN_GOLD_TAX_AMOUNT = 100;

bool is_ingr_chest(OBJ_DATA *obj);
void save_ingr_chests();
bool show_ingr_chest(OBJ_DATA *obj, CHAR_DATA *ch);
void save_chest_log();
// ���������� ���� �������
void tax_manage(CHAR_DATA *ch, std::string &buffer);
// ��������� ��������� ������� ������ ������
void init_xhelp();
/// ����������� � ������� ����-����� � gold ���
/// \return ����� ������������� ������, ������� ���� ����� � ����
long do_gold_tax(CHAR_DATA *ch, long gold);

} // namespace ClanSystem

#define POLITICS_NEUTRAL  0
#define POLITICS_WAR      1
#define POLITICS_ALLIANCE 2

#define HCE_ATRIUM 0
#define	HCE_PORTAL 1

// ������ ������ �� ����� (�����)
#define CHEST_UPDATE_PERIOD 10
// ������ ���������� � ������ ������� ����� (�����)
#define CHEST_INVOICE_PERIOD 60
// ������ ���������� ������ ����� � ���� ������ � ������ ������� ���������� �� ���� (�����)
#define CLAN_TOP_REFRESH_PERIOD 360
// �������� ����� � ����
#define CLAN_TAX 1000
// ����� �� ������� �� ���������� �� ��������� � ����
#define CLAN_STOREHOUSE_TAX 1000
// ������� ��������� ����� ������ (������) ��� ���������
#define CLAN_STOREHOUSE_COEFF 50

#define MAX_CLANLEVEL 5
// ����� ���� � ����������� ����-�����
#define CLAN_STUFF_ZONE 18

#define CHEST_IDENT_PAY 110

void fix_ingr_chest_rnum(const int room_rnum);//����� ���� ������� ������ �� �������

class ClanMember
{
public:
	ClanMember() :
		rank_num(0),
		money(0),
		exp(0),
		clan_exp(0),
		level(0),
		remort(false)
	{};

	std::string name;   // ��� ������
	int rank_num;       // ����� �����
	long long money;    // ������ ��������� �� ��������� � �������� �����
	long long exp;      // �������� ���-�����
	long long clan_exp; // ��������� ����-�����

/// �� ����������� � ����-�����
	// ������� ��� ���� ����� (��, ��� ������)
	int level;
	// ������� �������� ������ ��� ���� ��
	std::string class_abbr;
	// �� ����� ��� ���
	bool remort;
};

struct ClanPk
{
	long author;            // ��� ������
	std::string victimName;	// ��� ������
	std::string authorName;	// ��� ������
	time_t time;            // ����� ������
	std::string text;       // �����������
};

struct ClanStuffName
{
	int num;
	std::string name;
	std::string desc;
	std::string longdesc;
	std::vector<std::string> PNames;
};

class Clan;

typedef boost::shared_ptr<Clan> ClanPtr;
typedef std::vector<ClanPtr> ClanListType;
typedef boost::shared_ptr<ClanMember> ClanMemberPtr;
typedef std::map<long, ClanMemberPtr> ClanMemberList;
typedef boost::shared_ptr<ClanPk> ClanPkPtr;
typedef std::map<long, ClanPkPtr> ClanPkList;
typedef std::vector<std::bitset<ClanSystem::CLAN_PRIVILEGES_NUM> > ClanPrivileges;
typedef std::map<int, int> ClanPolitics;
typedef std::vector<ClanStuffName> ClanStuffList;

struct ClanOLC
{
	int mode;                  // ��� �������� ��������� ���
	ClanPtr clan;              // ����, ������� ������
	ClanPrivileges privileges; // ���� ������ ���������� �� ������ �� ���������� ��� ������
	int rank;                  // ������������� � ������ ������ ����
	std::bitset<ClanSystem::CLAN_PRIVILEGES_NUM> all_ranks; // ����� ��� ��������/���������� ���� ������
};

struct ClanInvite
{
	ClanPtr clan; // ������������ ����
	int rank;     // ����� �������������� �����
};


class Clan
{
public:
	Clan();
	~Clan();

	static ClanListType ClanList; // ������ ������

	static void ClanLoad();
	static void ClanSave();
	static void SaveChestAll();
	static void HconShow(CHAR_DATA * ch);
	static void SetClanData(CHAR_DATA * ch);
	static void ChestUpdate();
	static bool MayEnter(CHAR_DATA * ch, room_rnum room, bool mode);
	static bool InEnemyZone(CHAR_DATA * ch);
	static bool PutChest(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * chest);
	static bool TakeChest(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * chest);
	static void ChestInvoice();
	static bool BankManage(CHAR_DATA * ch, char *arg);
	static room_rnum CloseRent(room_rnum to_room);
	static ClanListType::const_iterator IsClanRoom(room_rnum room);
	static void CheckPkList(CHAR_DATA * ch);
	static void SyncTopExp();
	static int GetTotalCharScore(CHAR_DATA * ch);
	static int GetRankByUID(long);
	static bool ChestShow(OBJ_DATA * list, CHAR_DATA * ch);
	static void remove_from_clan(long unique);
	static int print_spell_locate_object(CHAR_DATA *ch, int count, std::string name);
	static int GetClanWars(CHAR_DATA * ch);
	static void init_chest_rnum();
	static bool is_clan_chest(OBJ_DATA *obj);
	bool is_clan_member(int unique);//���������� true ���� ��� � ������ unique � �����
	bool is_alli_member(int unique);//���������� true ���� ��� � ������ unique � �������
	static void clan_invoice(CHAR_DATA *ch, bool enter);
	static void save_pk_log();

	static bool put_ingr_chest(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *chest);
	static bool take_ingr_chest(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *chest);

	void Manage(DESCRIPTOR_DATA * d, const char * arg);
	void AddTopExp(CHAR_DATA * ch, int add_exp);

	const char * GetAbbrev()
	{
		return this->abbrev.c_str();
	};
	int GetRent();
	int GetOutRent();
	void SetClanExp(CHAR_DATA *ch, int add);
	int GetClanLevel()
	{
		return this->clan_level;
	};
	std::string GetClanTitle()
	{
		return this->title;
	};
	std::string get_abbrev() const
	{
		return abbrev;
	};
	std::string get_file_abbrev() const;
	bool CheckPrivilege(int rank, int privilege)
	{
		return this->privileges[rank][privilege];
	};
	int CheckPolitics(int victim);

	void add_remember(std::string text, int flag);
	std::string get_remember(unsigned int num, int flag) const;

	void write_mod(const std::string &arg);
	void print_mod(CHAR_DATA *ch) const;
	void load_mod();
	int get_rep();
	void set_rep(int rep);
	void init_ingr_chest();
	int get_ingr_chest_room_rnum() const { return ingr_chest_room_rnum_; };
	void set_ingr_chest_room_rnum(const int new_rnum) { ingr_chest_room_rnum_ = new_rnum; };
	int ingr_chest_tax();
	void purge_ingr_chest();
	int get_ingr_chest_objcount() const { return ingr_chest_objcount_; };
	bool ingr_chest_active() const;
	void set_ingr_chest(CHAR_DATA *ch);
	void disable_ingr_chest(CHAR_DATA *ch);

	int calculate_clan_tax() const;
	void add_offline_member(const std::string &name, int uid, int rank);
	int ingr_chest_max_objects();

	void save_chest();

	std::string get_web_url() const { return web_url_; };

	void set_bank(unsigned num);
	unsigned get_bank() const;

	void set_gold_tax_pct(unsigned num);
	unsigned get_gold_tax_pct() const;

	friend ACMD(DoHouse);
	friend ACMD(DoClanChannel);
	friend ACMD(DoClanList);
	friend ACMD(DoShowPolitics);
	friend ACMD(DoHcontrol);
	friend ACMD(DoWhoClan);
	friend ACMD(DoClanPkList);
	friend ACMD(DoStoreHouse);
	friend ACMD(do_clanstuff);
	friend ACMD(DoShowWars);

	// ���� ��
	ClanPkLog pk_log;
	// ��������� �� ��������� ����� �����
	ClanExp last_exp;
	// ���������� ������� ����� ��� ����� �������
	ClanExpHistory exp_history;
	// ��� ����-�����
	ClanChestLog chest_log;

private:
	std::string abbrev; // ������������ �����, ���� �����
	std::string name;   // ������� ��� �����
	std::string title;  // ��� ����� ����� � ������ ������ ����� (����� ���.�����, ���� ��� �� ������������)
	std::string title_female; // title ��� ���������� �������� ����
	std::string owner;  // ��� �������
	mob_vnum guard;     // �������� �����
	time_t builtOn;     // ���� ��������
	double bankBuffer;  // ������ ��� ����� ������� ������ �� ���������
	bool entranceMode;  // ���� � ����� ��� ����/������ ���� � ������
	std::vector <std::string> ranks; // ������ �������� ������
	std::vector <std::string> ranks_female; // ������ �������� ������ ��� �������� ����
	ClanPolitics politics;     // ��������� ��������
	ClanPkList pkList;  // ������
	ClanPkList frList;  // ������
	long bank;          // ��������� ����� �����
	long long exp; // ��������� ���-�����
	long long clan_exp; //��������� ����-�����
	long exp_buf;  // ������ ��� ��������� ���-����� � ������ ������� �������� � ���-����� (exp_info), ������������� ��� � 6 �����
	int clan_level; // ������� ������� �����
	int rent;       // ����� ����������� ������� � �����, ������ ��� �����
	int out_rent;   // ����� ���� ��� ����������, ����� �� ���������� � ����� ������
	int chest_room; // ������� � ��������, �� ������� ��������� �����. ����� �� ������ �������� ���� � ������
	ClanPrivileges privileges; // ������ ���������� ��� ������
	ClanMemberList members;    // ������ ������ ������� (���, ���, ����� �����)
	ClanStuffList clanstuff;   // ����-����
	bool storehouse;    // ����� ������� �� ��������� �� ���������� �����
	bool exp_info;      // ���������� ��� ��� ��������� �����
	bool test_clan;     // �������� ���� (������ ���)
	std::string mod_text; // ��������� �������
	// ���� �������, ��� ����� ��������� ��� ����� (-1 ���� ����� ���������)
	int ingr_chest_room_rnum_;
	// ����� ����� ������� ��� '������� �����������'
	std::string web_url_;
	// ���� ����� �� ���� ����� �� ��� ���
	unsigned gold_tax_pct_;
	// ���� ���������
	int reputation;
	//no save
	int chest_objcount;
	int chest_discount;
	int chest_weight;
	Remember::RememberListType remember_; // ��������� ����
	Remember::RememberListType remember_ally_; // ��������� ����
	int ingr_chest_objcount_;

	void ClanUpgrade();
	void SetPolitics(int victim, int state);
	void ManagePolitics(CHAR_DATA* ch, std::string& buffer);
	void HouseInfo(CHAR_DATA* ch);
	void HouseAdd(CHAR_DATA* ch, std::string& buffer);
	void HouseRemove(CHAR_DATA* ch, std::string& buffer);
	void ClanAddMember(CHAR_DATA* ch, int rank);
	void HouseOwner(CHAR_DATA* ch, std::string& buffer);
	void HouseLeave(CHAR_DATA* ch);
	int GetClanScore();
	void HouseStat(CHAR_DATA* ch, std::string& buffer);
	void remove_member(ClanMemberList::iterator& it);
	void save_clan_file(const std::string& filename) const;
	void house_web_url(CHAR_DATA* ch, const std::string& buffer);

	// house ��� ���
	void MainMenu(DESCRIPTOR_DATA * d);
	void PrivilegeMenu(DESCRIPTOR_DATA * d, unsigned num);
	void AllMenu(DESCRIPTOR_DATA * d, unsigned flag);
	void GodToChannel(CHAR_DATA *ch, std::string text, int subcmd);
	void CharToChannel(CHAR_DATA *ch, std::string text, int subcmd);

	static void HcontrolBuild(CHAR_DATA * ch, std::string & buffer);
	static void HcontrolDestroy(CHAR_DATA * ch, std::string & buffer);
	static void hcontrol_title(CHAR_DATA *ch, std::string &text);
	static void hcontrol_rank(CHAR_DATA *ch, std::string &text);
	static void hcontrol_exphistory(CHAR_DATA *ch, std::string &text);
	static void hcontrol_set_ingr_chest(CHAR_DATA *ch, std::string &text);
	static void hcon_outcast(CHAR_DATA *ch, std::string &buffer);
	static void hcon_owner(CHAR_DATA *ch, std::string &text);

	static void ChestLoad();
	int ChestTax();
	int ChestMaxObjects()
	{
		return (this->clan_level + 1)*500 + 100;
	};
	int ChestMaxWeight()
	{
		return (this->clan_level + 1)*5000 + 500;
	};

	// ��� ���������� ������ ������ ����� �� ������, ����� ��� ����� ���� ���� ���������
	class SortRank
	{
	public:
		bool operator()(const CHAR_DATA * ch1, const CHAR_DATA * ch2);
	};
};

void SetChestMode(CHAR_DATA *ch, std::string &buffer);
std::string GetChestMode(CHAR_DATA *ch);
std::string clan_get_custom_label(OBJ_DATA *obj, ClanPtr clan);

#endif

// vim: ts=4 sw=4 tw=0 noet syntax=cpp :
