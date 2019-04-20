#include <graphenelib/action.h>
#include <graphenelib/asset.h>
#include <graphenelib/contract.hpp>
#include <graphenelib/contract_asset.hpp>
#include <graphenelib/crypto.h>
#include <graphenelib/dispatcher.hpp>
#include <graphenelib/global.h>
#include <graphenelib/multi_index.hpp>
#include <graphenelib/print.hpp>
#include <graphenelib/system.h>
#include <graphenelib/types.h>


using namespace graphene;

class auction : public contract{
    
    public:
    auction(uint64_t id)
    : contract(id),
    priiterms(_self, _self),
    pubiterms(_self, _self),
    itermidraws(_self, _self),
    personlists(_self, _self)
    {
    }
    
    ///@abi action
    void signin(){
        uint64_t sender = get_trx_sender();
        auto iter = personlists.find(sender);
        graphene_assert(iter == personlists.end(), "Repeat registration.\n");
        personlists.emplace(0, [&](auto &o){
            o.personid = sender;
            o.credit = 60;
        });
    }
    
    ///@abi action
    void verifyseller(uint64_t min_price, std::string name, std::string discreption)
    {
        uint64_t status = 1;

        graphene_assert(status == 1, "verify failed!");
        
        auto iter = itermidraws.find(1);
        if(iter == itermidraws.end()){
            itermidraws.emplace(0, [&](auto &o){
            o.pk = 1;
            o.itermid = 1;
            });
            iter = itermidraws.find(1);
        }
        auto sender = get_trx_sender();
            uint64_t Itermid = iter->itermid;
            auto test = personlists.find(sender);
            graphene_assert(test != personlists.end(), "Please register first.\n");
            itermidraws.modify(iter, 0, [&](auto &o){
                o.itermid = o.itermid + 1;
            });
            Itermid = Itermid + 1;
            pubiterms.emplace(sender, [&](auto &o)
            {
                o.itermid = Itermid;
                o.name = name;
                o.discreption = discreption;
                o.status = 1;
                o.final_price = 0;
                o.encrpted_goods = "";
                o.last_pushtime = get_head_block_time();
                o.receive_time = get_head_block_time();

            });
            priiterms.emplace(sender, [&](auto &o)
            {
                o.itermid = Itermid;
                o.owner.sellerid = get_trx_sender();
                o.owner.min_price = min_price * 100000;
                o.owner.deposit = 0;
                o.buyer = 0;
                o.all_buyers = {};
                o.last_pusher = 0;  
            });
    }
    
    ///@abi action
    ///@abi payable
    void paydeposit(uint64_t itermid)
    {
        uint64_t sender = get_trx_sender();
        auto test = personlists.find(sender);
        graphene_assert(test != personlists.end(), "Please register first.\n");
        uint64_t Sellerid;
        uint64_t deposit = get_action_asset_amount();
        auto itermid_iter = priiterms.find(itermid);
        graphene_assert(deposit > itermid_iter->owner.min_price * 0.1 * (100-test->credit)/100, "Not enough money");
        if (itermid_iter != priiterms.end())
        {
            Sellerid = itermid_iter->owner.sellerid;
        }
        graphene_assert(sender == Sellerid, "invalid account");

        auto iterm_iter = priiterms.find(itermid);
        if (iterm_iter != priiterms.end())
        {
            priiterms.modify(iterm_iter, 0, [&](auto &o)
            {
                o.owner.deposit = deposit;
            });
        }
    }

    
    ///@abi action
    void joinrequest(uint64_t itermid){
        uint64_t sender = get_trx_sender();
        auto test = personlists.find(sender);
        graphene_assert(test != personlists.end(), "Please register first.\n");
        uint64_t owner;
        auto priiter = priiterms.find(itermid);
        auto pubiter = pubiterms.find(itermid);
        graphene_assert(pubiter->status == 1, "This iterm is not avaliable to sell now!\n");
        priiterms.modify(priiter, 0, [&](auto &o){
            o.all_buyers.push_back({sender, 0, 0});
        });
    }
    
    ///@abi action
    ///@abi payable
    void newprice(uint64_t itermid, uint64_t amount){
        uint64_t sender = get_trx_sender();
        auto test = personlists.find(sender);
        graphene_assert(test != personlists.end(), "Please register first.\n");
        auto priiter = priiterms.find(itermid);
        auto pubiter = pubiterms.find(itermid);
        auto now1 = get_head_block_time();
        int64_t wait_time = 100;
        graphene_assert(now1 > pubiter->receive_time + wait_time, "This item does not accept new price requests now.\n");
        uint64_t min_deposit = amount * 100000 * 0.1 * (100 - test->credit )/100;
        uint64_t deposit = get_action_asset_amount();
        graphene_assert(deposit > min_deposit, "You do not provide enough deposit.\n");
        graphene_assert(priiter->owner.min_price < amount * 100000, "Every new price must be greater than minmum price.\n");
        graphene_assert(pubiter->final_price < amount * 100000, "Every new price must be greater than the new price raised last time.\n");
        int64_t now = get_head_block_time();
        int64_t max_interval = 3600;
        graphene_assert(now < pubiter->last_pushtime + max_interval, "Auction ended.\n");
        uint64_t flag = 0;
        uint64_t last_pusher = priiter->last_pusher;
        uint64_t last_pusher_deposit = 0;
        priiterms.modify(priiter, 0, [&](auto &o){
        for(auto iterr = o.all_buyers.begin(); iterr != o.all_buyers.end(); iterr++){
            if(iterr->buyerid == sender){
                iterr->price = amount * 100000;
                iterr->deposit = deposit;
                flag = 1;
            }
            if(iterr->buyerid == last_pusher){
                last_pusher_deposit = iterr->deposit;
            }
        }
            graphene_assert(flag == 1, "You have to join this auction first.\n");
            o.buyer = sender;
            o.last_pusher = sender;
        });
        pubiterms.modify(pubiter, 0, [&](auto &o){
            o.final_price = amount * 100000;
            o.last_pushtime = get_head_block_time();
        });
        if(last_pusher_deposit != 0)
            withdraw_asset(_self, last_pusher, get_asset_id("GXC", 3), last_pusher_deposit);
        
    }
    
    ///@abi action
    void submitgood(std::string encrpted_goods, uint64_t itermid){
        uint64_t sender = get_trx_sender();
        auto test = personlists.find(sender);
        graphene_assert(test != personlists.end(), "Please register first.\n");
        auto pubiter = pubiterms.find(itermid);
        auto priiter = priiterms.find(itermid);
        auto now = get_head_block_time();
        int64_t max_interval = 100;
        graphene_assert(pubiter->status == 3, "Please wait buyer submitting money.\n");
        graphene_assert(now > pubiter->last_pushtime + max_interval, "It's not proper time to submit good.\n");
        graphene_assert(sender == priiter->owner.sellerid, "Authorization failed.\n");
        pubiterms.modify(pubiter, 0, [&](auto &o){
            o.encrpted_goods = encrpted_goods;
            o.status = 4;
        });
        if(test->credit < 80){
            personlists.modify(test, 0 , [&](auto &o){
                o.credit = o.credit + 2;
            });
        }
        else if(test->credit <= 99){
            personlists.modify(test, 0 , [&](auto &o){
                o.credit = o.credit + 1;
            });
        }
            
    }
    
    ///@abi action
    ///@abi payable
    void submitmoney(uint64_t itermid){
        uint64_t sender = get_trx_sender();
        auto test = personlists.find(sender);
        graphene_assert(test != personlists.end(), "Please register first.\n");
        auto pubiter = pubiterms.find(itermid);
        auto priiter = priiterms.find(itermid);
        uint64_t payment = get_action_asset_amount();
        graphene_assert(sender == priiter->buyer, "Authorization Failed.\n");
        graphene_assert(payment == pubiter->final_price, "Not enough money.\n");
        pubiterms.modify(pubiter, 0, [&](auto &o){
            o.status = 3;
        });
        if(test->credit < 80){
            personlists.modify(test, 0 , [&](auto &o){
                o.credit = o.credit + 2;
            });
        }
        else if(test->credit <= 99){
            personlists.modify(test, 0 , [&](auto &o){
                o.credit = o.credit + 1;
            });
        }
        for(auto iter = priiter->all_buyers.begin(); iter != priiter->all_buyers.end(); iter++){
            if(iter->buyerid == sender)
                withdraw_asset(_self, sender, get_asset_id("GXC", 3), iter->deposit);
        }
    }
    
    ///@abi action
    void withdraw(uint64_t itermid){
        uint64_t sender = get_trx_sender();
        auto test = personlists.find(sender);
        graphene_assert(test != personlists.end(), "Please register first.\n");
        auto pubiter = pubiterms.find(itermid);
        auto priiter = priiterms.find(itermid);
        graphene_assert(sender == priiter->owner.sellerid, "You are not allowed to withdraw.\n");
        graphene_assert(pubiter->status == 4, "You have already withdrawn the money.\n");
        uint64_t total_payment = pubiter->final_price * 0.9 + priiter->owner.deposit;
        pubiterms.modify(pubiter, 0, [&](auto &o){
            o.status = o.status + 1;
        });
        withdraw_asset(_self, sender, get_asset_id("GXC", 3), total_payment);
    }

    ///@abi action
    void withdrawpro(uint64_t amount){
        uint64_t sender = get_trx_sender();
        graphene_assert(sender == authority, "You are not allowed to withdraw these money.\n");
        auto contract_id = current_receiver();
        auto balance = get_balance(contract_id, get_asset_id("GXC", 3));
        graphene_assert(balance > amount * 100000, "Not enough money.\n");
        withdraw_asset(_self, sender, get_asset_id("GXC", 3), amount * 100000);
    }
    
    ///@abi action
    void clearall(uint64_t itermid){
        auto pubiter = pubiterms.find(itermid);
        if(pubiter != pubiterms.end())
            pubiterms.erase(pubiter);
        auto priiter = priiterms.find(itermid);
        if(priiter != priiterms.end())
            priiterms.erase(priiter);
    }
    
    ///@abi action
    void buyertout(uint64_t itermid){
        if(get_head_block_num() % 10 == 0){
            auto sender = get_trx_sender();
            auto test = personlists.find(sender);
            graphene_assert(test != personlists.end(), "Please register first.\n");
            auto priiter = priiterms.find(itermid);
            auto pubiter = pubiterms.find(itermid);
            auto now = get_head_block_time();
            int64_t maxinterval = 1000;
            if(pubiter->last_pushtime + maxinterval < now){
                pubiterms.modify(pubiter, 0, [&](auto &o){
                    o.status = 10;
                });
                for(auto iter = priiter->all_buyers.begin(); iter != priiter->all_buyers.end(); iter++){
                    if(iter->buyerid == priiter->buyer){
                        withdraw_asset(_self, sender, get_asset_id("GXC", 3), 0.05*iter->deposit);
                    }
                }  
            }
        }
    }
    
    ///@abi action
    void sellertout(uint64_t itermid){
        if(get_head_block_num() % 10 == 0){
            auto sender = get_trx_sender();
            auto test = personlists.find(sender);
            graphene_assert(test != personlists.end(), "Please register first.\n");
            auto priiter = priiterms.find(itermid);
            auto pubiter = pubiterms.find(itermid);
            auto now = get_head_block_time();
            int64_t maxinterval = 1000;
            if(pubiter->last_pushtime + maxinterval < now){
                pubiterms.modify(pubiter, 0, [&](auto &o){
                    o.status = 11;
                });
                withdraw_asset(_self, sender, get_asset_id("GXC", 3), priiter->owner.deposit * 0.05); 
            }
        }
    }
    
    //@abi table pubiterm i64
    struct pubiterm{
        uint64_t itermid;
        std::string name;
        std::string discreption;
        std::string encrpted_goods;
        uint16_t status;
        uint64_t final_price;
        int64_t last_pushtime;
        int64_t receive_time;
    
        uint64_t primary_key() const { return itermid; }
    
        GRAPHENE_SERIALIZE(pubiterm, 
            (itermid)(name)(discreption)(encrpted_goods)(status)(final_price)(last_pushtime)(receive_time))
    };
    
    typedef multi_index<N(pubiterm), pubiterm> pubiterms_index;
    
    pubiterms_index pubiterms;
    
   //@abi table itermidraw i64
   struct itermidraw{
       uint64_t pk;
       uint64_t itermid;
        
        uint64_t primary_key() const {return pk; }
        
        GRAPHENE_SERIALIZE(itermidraw, (pk)(itermid))
   };
    
    typedef multi_index<N(itermidraw), itermidraw> itermidraw_index;
    itermidraw_index itermidraws;
   
    
    struct buyer{
      uint64_t buyerid;
      uint64_t price;
      uint64_t deposit;
      
      GRAPHENE_SERIALIZE(buyer, (buyerid)(price)(deposit))
    };
    
    struct seller{
      uint64_t sellerid;
      uint64_t min_price;
      uint64_t deposit;
      
      GRAPHENE_SERIALIZE(seller, (sellerid)(min_price)(deposit))
    };
    
    //@abi table priiterm i64
    struct priiterm{
    uint64_t itermid;
    struct seller owner;
    uint64_t buyer;
    std::vector<struct buyer> all_buyers;
    uint64_t last_pusher;
    
    uint64_t primary_key() const { return itermid; }
    
    GRAPHENE_SERIALIZE(priiterm, 
            (itermid)(owner)(buyer)(all_buyers)(last_pusher))
    };

    typedef multi_index<N(priiterm), priiterm> priiterms_index;
    
    priiterms_index priiterms;
    
    const uint64_t authority = 2627;
    
    
    //@abi table personlist i64
    struct personlist{
        uint64_t personid;
        uint64_t credit;
        
        uint64_t primary_key() const {return personid;}
        
        GRAPHENE_SERIALIZE(personlist, (personid)(credit))
    };
    
    typedef multi_index<N(personlist), personlist> personlist_index;
    
    personlist_index personlists;
    
    
};


GRAPHENE_ABI(auction, (signin)(verifyseller)(paydeposit)(joinrequest)(newprice)(submitgood)(submitmoney)(withdraw)(withdrawpro)(clearall)(buyertout)(sellertout))