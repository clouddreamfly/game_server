/*
 *  Created on: Dec 21, 2015
 *      Author: zhangyalei
 */

#include "Common_Func.h"
#include "Game_Timer.h"
#include "Game_Manager.h"
#include "Game_Server.h"
#include "Game_Connector.h"
#include "Log_Connector.h"
#include "Msg_Manager.h"
#include "Scene_Manager.h"
#include "AI_Manager.h"

Game_Manager::Game_Manager(void):
  server_id_(0) { }

Game_Manager::~Game_Manager(void) {}

Game_Manager *Game_Manager::instance_;

Game_Manager *Game_Manager::instance(void) {
	if (instance_ == 0)
		instance_ = new Game_Manager;
	return instance_;
}

int Game_Manager::init(int server_id) {
	server_id_ = server_id;
	set_server_name("Game_Server");
	AI_MANAGER->init();
	GAME_TIMER->thr_create();
	MSG_MANAGER->init();
	return 0;
}

int Game_Manager::unbind_player(Player &player) {
	Server_Manager::unbind_player(player);
	player_cid_map_.erase(GET_CID(player.gate_cid(), player.player_cid()));
	return 0;
}

int Game_Manager::send_to_gate(int gate_cid, Block_Buffer &buf) {
	if (gate_cid < 2) {
		LOG_ERROR("gate_cid = %d", gate_cid);
		return -1;
	}
	return GAME_GATE_SERVER->send_block(gate_cid, buf);
}

int Game_Manager::send_to_master(Block_Buffer &buf) {
	int master_cid = GAME_MASTER_CONNECTOR->get_cid();
	if (master_cid < 2) {
		LOG_ERROR("master_cid = %d", master_cid);
		return -1;
	}
	return GAME_MASTER_CONNECTOR->send_block(master_cid, buf);
}

int Game_Manager::send_to_db(Block_Buffer &buf) {
	int db_cid = GAME_DB_CONNECTOR->get_cid();
	if (db_cid < 2) {
		LOG_ERROR("db_cid = %d", db_cid);
		return -1;
	}
	return GAME_DB_CONNECTOR->send_block(db_cid, buf);
}

int Game_Manager::send_to_log(Block_Buffer &buf) {
	int log_cid = LOG_CONNECTOR->get_cid();
	if (log_cid < 2) {
		LOG_ERROR("db_cid = %d", log_cid);
		return -1;
	}
	return LOG_CONNECTOR->send_block(log_cid, buf);
}

int Game_Manager::close_client(int gate_cid, int player_cid, int error_code) {
	Block_Buffer buf;
	buf.make_player_message(ACTIVE_DISCONNECT, error_code, player_cid);
	buf.finish_message();
	return send_to_gate(gate_cid, buf);
}

int Game_Manager::process_list(void) {
	int32_t cid = 0;

	while (1) {
		bool all_empty = true;

		//掉线玩家列表
		if (! drop_gate_cid_list_.empty()) {
			all_empty = false;
			cid = drop_gate_cid_list_.pop_front();
			process_drop_gate_cid(cid);
		}
		//定时器列表
		if (! tick_list_.empty()) {
			all_empty = false;
			tick_list_.pop_front();
			tick();
		}

		if (all_empty)
			Time_Value::sleep(Time_Value(0,100));
	}
	return 0;
}

void Game_Manager::process_drop_gate_cid(int gate_cid) {
	for (Player_Cid_Map::iterator iter = player_cid_map_.begin(); iter != player_cid_map_.end(); ) {
		if (iter->second->gate_cid() == gate_cid) {
			iter->second->link_close();
			player_cid_map_.erase(iter++);
		}
	}
}

void Game_Manager::get_server_info(void) {
	game_gate_server_info_.reset();
	GAME_GATE_SERVER->get_server_info(game_gate_server_info_);
}

void Game_Manager::free_cache(void) {
	Server_Manager::free_cache();
	player_pool_.shrink_all();
	GAME_GATE_SERVER->free_cache();
	GAME_DB_CONNECTOR->free_cache();
	GAME_MASTER_CONNECTOR->free_cache();
}

void Game_Manager::print_object_pool(void) {
	Server_Manager::print_object_pool();
	LOG_INFO("Game_Server player_pool_ free = %d, used = %d", player_pool_.free_obj_list_size(), player_pool_.used_obj_list_size());
}
