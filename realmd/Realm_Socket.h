#pragma once
#ifndef REALM_SOCKET_H
#define REALM_SOCKET_H

#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Refcounted_Auto_Ptr.h>
#include <string>
#include <list>
#include <map>
#include <openssl/bn.h>


#include "Common.h"

class ByteBuffer;

namespace Trinity
{

class Account
{
 public:
  std::string sha_pass;
  uint64 id;
  bool locked;
  std::string last_ip;
  uint8 gmlevel;
  uint8 failed_logins;
};

enum AccountState
{
  ACCOUNT_EXISTS,
  ACCOUNT_NOTFOUND,
  ACCOUNT_BANNED
};


class Realm_Socket : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{

 public:
  Realm_Socket();
  ~Realm_Socket();
  int open(void*);
  int close(u_long flags);
  const std::string& get_ip(){return ip;}
  const std::string& get_login(){return login;}
  void ip_ban_checked(bool);
  void account_checked(AccountState);
  void send(ByteBuffer*);
  Account acct;
  void get_char_amount(std::map<uint8, uint8>);
 private:
  std::map<uint8, uint8> realm_char_amount;
  bool out_active;
  void die();
  void handle_auth_logon_challenge();
  void handle_auth_logon_proof();
  void handle_realm_list();
  void set_vs();
  int handle_input(ACE_HANDLE);
  int handle_output(ACE_HANDLE);
  int handle_close(ACE_HANDLE, ACE_Reactor_Mask);
  ACE_Refcounted_Auto_Ptr<Realm_Socket, ACE_Recursive_Thread_Mutex> ptr;
  char raw_buf[4096];
  uint16 client_build;
  std::string login;
  std::string ip;
  std::list<ByteBuffer*> packet_queue;
  ACE_Recursive_Thread_Mutex queue_mtx;
  BN_CTX* ctx;
  BIGNUM* s, *v, *g, *k, *N, *b, *B;
};

typedef ACE_Refcounted_Auto_Ptr<Realm_Socket, ACE_Recursive_Thread_Mutex> Realm_Sock_Ptr;

};
#else
#endif
