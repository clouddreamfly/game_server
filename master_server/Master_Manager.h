/*
 *  Created on: Dec 21, 2015
 *      Author: zhangyalei
 */

#ifndef MASTER_MANAGER_H_
#define MASTER_MANAGER_H_

#include "Server_Manager.h"
#include "Master_Player.h"

class Master_Manager: public Server_Manager {
	typedef Object_Pool<Master_Player, Spin_Lock> Player_Pool;
public:
	static Master_Manager *instance(void);
	int init(void);

	inline Master_Player *pop_player(void) { return player_pool_.pop(); }
	inline int push_player(Master_Player *player) { return player_pool_.push(player); }

	int bind_game_cid_player(int cid, Player *player);
	Player *find_game_cid_player(int cid);
	virtual int unbind_player(Player &player);

	//发送数据
	int send_to_gate(int gate_cid, Block_Buffer &buf);
	int send_to_game(int game_cid, Block_Buffer &buf);
	int send_to_db(Block_Buffer &buf);
	int send_to_log(Block_Buffer &buf);
	int send_to_http(int http_cid, Block_Buffer &buf);

	//关闭客户端连接
	int close_client(int gate_cid, int player_cid, int error_code);

	//消息处理
	int process_list();
	int push_tick(int x);
	int push_master_gate_data(Block_Buffer *buf);
	Block_Buffer* pop_master_gate_data();
	int push_master_game_data(Block_Buffer *buf);
	Block_Buffer* pop_master_game_data();
	int push_master_db_data(Block_Buffer *buf);
	Block_Buffer* pop_master_db_data();
	int push_master_http_data(Block_Buffer *buf);
	Block_Buffer* pop_master_http_data();
	int push_drop_player_cid(int cid);
	int pop_drop_player_cid(void);

	virtual void get_server_info(void);
	virtual void free_cache(void);
	virtual void print_object_pool(void);

	void load_master_db_data();

private:
	Master_Manager(void);
	virtual ~Master_Manager(void);
	Master_Manager(const Master_Manager &);
	const Master_Manager &operator=(const Master_Manager &);

private:
	static Master_Manager *instance_;

	Player_Pool player_pool_;
	Player_Cid_Map player_game_cid_map_;;

	Int_List tick_list_;									//定时器列表
	Data_List master_gate_data_list_;		//gate-->master
	Data_List master_game_data_list_;		//game-->master
	Data_List master_db_data_list_;			//db-->master
	Data_List master_http_data_list_;		//http-->master
	Int_List drop_player_cid_list_;			//掉线的玩家cid列表

	Server_Info master_gate_server_info_;
	Server_Info master_game_server_info_;
};

#define MASTER_MANAGER Master_Manager::instance()

////////////////////////////////////////////////////////////////////////////////
inline int Master_Manager::push_tick(int x) {
	tick_list_.push_back(x);
	return 0;
}

inline int Master_Manager::push_master_gate_data(Block_Buffer *buf) {
	return master_gate_data_list_.push_back(buf);
}

inline Block_Buffer* Master_Manager::pop_master_gate_data() {
	return master_gate_data_list_.pop_front();
}

inline int Master_Manager::push_master_game_data(Block_Buffer *buf) {
	return master_game_data_list_.push_back(buf);
}

inline Block_Buffer* Master_Manager::pop_master_game_data() {
	return master_game_data_list_.pop_front();
}

inline int Master_Manager::push_master_db_data(Block_Buffer *buf){
	return master_db_data_list_.push_back(buf);
}

inline Block_Buffer *Master_Manager::pop_master_db_data(void){
	return master_db_data_list_.pop_front();
}

inline int Master_Manager::push_master_http_data(Block_Buffer *buf){
	return master_http_data_list_.push_back(buf);
}

inline Block_Buffer *Master_Manager::pop_master_http_data(void){
	return master_http_data_list_.pop_front();
}

inline int Master_Manager::push_drop_player_cid(int cid) {
	drop_player_cid_list_.push_back(cid);
	return 0;
}

inline int Master_Manager::pop_drop_player_cid(void) {
	if (drop_player_cid_list_.empty()) {
		return 0;
	}
	return drop_player_cid_list_.pop_front();
}

#endif /* MASTER_MANAGER_H_ */
