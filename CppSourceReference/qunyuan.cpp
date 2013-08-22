
#include "qunyuan.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "maneuvering.h"
#include "settings.h"



EOSCard::EOSCard(){
    once = true;
    will_throw = false;
}

bool EOSCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;
    return to_select->hasArmor() || to_select->hasDefhorse() || to_select->hasOffhorse();
}

void EOSCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *jinteng = effect.from;
    Room *room = jinteng->getRoom();    

    QString choice = room->askForChoice(effect.from, "EOS", "fangju+fangyuma+jingongma");

    if(choice == "fangju"){
        if(effect.to->hasArmor()){
            room->moveCardTo(this, effect.to, Player::Hand);
            room->moveCardTo(effect.to->getEquip(1), jinteng, Player::Hand);
        }
    }else if(choice == "fangyuma"){
        if(effect.to->hasDefhorse()){
            room->moveCardTo(this, effect.to, Player::Hand);
            room->moveCardTo(effect.to->getEquip(2), jinteng, Player::Hand);
        }
    }else{
        if(effect.to->hasOffhorse()){
            room->moveCardTo(this, effect.to, Player::Hand);
            room->moveCardTo(effect.to->getEquip(3), jinteng, Player::Hand);
        }
    }

    room->playSkillEffect("EOS");

    return;


}


class EOS: public OneCardViewAsSkill{
public:
    EOS():OneCardViewAsSkill("EOS"){

    }


    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("EOSCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getTypeId() == Card::Equip;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        EOSCard *eos_card = new EOSCard();
        eos_card->addSubcard(card_item->getFilteredCard());
        return eos_card;
    }
};


ZibaoCard::ZibaoCard(){
    once = true;
}

bool ZibaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;
    return Self->distanceTo(to_select) <= 2;
}


void ZibaoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->playSkillEffect("zibao");
    room->loseMaxHp(effect.from);
    room->loseHp(effect.to);
}


class Zibao: public ZeroCardViewAsSkill{
public:
    Zibao():ZeroCardViewAsSkill("zibao"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZibaoCard");
    }

    virtual const Card *viewAs() const{
        return new ZibaoCard;
    }
};


static bool CompareBySuit(int card1, int card2){
    const Card *c1 = Sanguosha->getCard(card1);
    const Card *c2 = Sanguosha->getCard(card2);

    int a = static_cast<int>(c1->getSuit());
    int b = static_cast<int>(c2->getSuit());

    return a < b;
}


class Juesi: public TriggerSkill{
public:
    Juesi():TriggerSkill("juesi"){
        events << Dying << AskForPeachesDone;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        Room *room = player->getRoom();
        return player == room->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jim, QVariant &data) const{
        Room *room = jim->getRoom();

        if(event == Dying){
            if(jim && jim->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect("juesi");
                int x = jim->getMaxHP();
                if(x > 4)
                    x = 4;
                bool has_duplicate = false;
                bool maxhp_increased = false;
                bool has_second_duplicate = false;
                bool survived = false;

                QList<int> card_ids = room->getNCards(x);
                int i, j;
                for(i=0; i < card_ids.length(); i++){
                    for(j=i+1; j < card_ids.length(); j++){
                        const Card *card = Sanguosha->getCard(card_ids.at(i));
                        if(card->sameColorWith( Sanguosha->getCard(card_ids.at(j)) )){
                            has_duplicate = true;
                        }
                    }
                }
                qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
                room->fillAG(card_ids);
                if(has_duplicate)
                    if(room->askForChoice(jim, "juesi", "maxhp+cancel") == "maxhp"){

                        int card_id = room->askForAG(jim, card_ids, false, "juesi");
                        card_ids.removeOne(card_id);
                        room->takeAG(NULL, card_id);

                        int card_id2 = room->askForAG(jim, card_ids, false, "juesi");
                        card_ids.removeOne(card_id2);
                        room->takeAG(NULL, card_id2);

                        if(Sanguosha->getCard(card_id)->sameColorWith(Sanguosha->getCard(card_id2))){
                            room->setPlayerProperty(jim, "maxhp", jim->getMaxHP() + 1);
                            maxhp_increased = true;
                        }
                    }

                for(i=0; i < card_ids.length(); i++){
                    for(j=i+1; j < card_ids.length(); j++){
                        const Card *card = Sanguosha->getCard(card_ids.at(i));
                        if(card->sameColorWith( Sanguosha->getCard(card_ids.at(j)) )){
                            has_second_duplicate = true;
                        }
                    }
                }

                if(maxhp_increased && has_second_duplicate)
                    if(room->askForChoice(jim, "juesi", "hp+cancel") == "hp"){

                        int card_id = room->askForAG(jim, card_ids, false, "juesi");
                        card_ids.removeOne(card_id);
                        room->takeAG(NULL, card_id);

                        int card_id2 = room->askForAG(jim, card_ids, false, "juesi");
                        card_ids.removeOne(card_id2);
                        room->takeAG(NULL, card_id2);

                        if(Sanguosha->getCard(card_id)->sameColorWith(Sanguosha->getCard(card_id2))){
                            RecoverStruct recover;
                            recover.who = jim;
                            room->recover(jim, recover);
                            if(jim->getHp()>0)
                                survived = true;
                        }
                    }

                if(survived){
                    foreach(int card_id, card_ids){
                        room->takeAG(jim, card_id);
                    }
                }else{
                    foreach(int card_id, card_ids){
                        jim->addToPile("juesi", card_id);
                    }
                }
                room->broadcastInvoke("clearAG");

                return false;
            }

        }else if(event == AskForPeachesDone){
            if(jim->isAlive()){
                const QList<int> juesi(jim->getPile("juesi"));

                foreach(int card_id, juesi){
                    jim->obtainCard(Sanguosha->getCard(card_id));
                }
            }
        }

        return false;
    }
};


class Fuqin: public TriggerSkill{
public:
    Fuqin():TriggerSkill("fuqin"){
        events << CardLost << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huanying, QVariant &data) const{
        if(huanying->getPhase() != Player::Discard)
            return false;

        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile){
                huanying->addMark("fuqin");
                if(huanying->getMark("fuqin") == 2 && huanying->isWounded()){
                    if(huanying->askForSkillInvoke(objectName())){
                        huanying->getRoom()->playSkillEffect(objectName(), 2);
                        RecoverStruct recover;
                        recover.who = huanying;
                        huanying->getRoom()->recover(huanying, recover);
                    }
                }
            }
        }else if(event == PhaseChange){
            huanying->setMark("fuqin", 0);
        }

        return false;
    }
};

class YiqiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return !player->hasEquip(card) && card->getTypeId() == Card::Trick;
    }

    virtual bool willThrow() const{
        return false;
    }
};

YiqiCard::YiqiCard(){
    once = true;
}

bool YiqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void YiqiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    const Card *card = room->askForCardShow(effect.to, effect.from, "yiqi");
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->getTypeId() == Card::Trick){
        const Card *showcard = room->askForCard(effect.from, ".yiqi", "@yiqi");
        if(showcard){
            effect.to->obtainCard(showcard);
        }else{
            effect.to->drawCards(1, false);
        }

    }else{
        effect.from->obtainCard(card);
    }
}

class Yiqi: public ZeroCardViewAsSkill{
public:
    Yiqi():ZeroCardViewAsSkill("yiqi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YiqiCard");
    }

    virtual const Card *viewAs() const{
        return new YiqiCard;
    }
};


class Zhongguo: public TriggerSkill{
public:
    Zhongguo():TriggerSkill("zhongguo"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *onefourfour, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!onefourfour->isNude() && !onefourfour->hasEquipweapon()){
            Room *room = onefourfour->getRoom();
            if(onefourfour->askForSkillInvoke(objectName(), data)){
                const Card *card = room->askForCard(effect.from, "..", "@zhongguo");
                room->throwCard(card);
                room->slashResult(effect, NULL);
            }
        }

        return false;
    }
};


class ChongqiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->getTypeId() == Card::Equip;
    }
};


ChongqiCard::ChongqiCard(){
    target_fixed = true;
}

void ChongqiCard::use(Room *room, ServerPlayer *onefourfour, const QList<ServerPlayer *> &) const{
    const Card *equip = room->askForCard(onefourfour, ".chongqi", "@chongqi");
        if(equip){
            room->throwCard(equip);
            room->playCardEffect("@recast", onefourfour->getGeneral()->isMale());
            onefourfour->drawCards(1, false);
        }
}

class Chongqi:public ZeroCardViewAsSkill{
public:
    Chongqi():ZeroCardViewAsSkill("chongqi"){
    }

    virtual const Card *viewAs() const{
        return new ChongqiCard;
    }
};


class QiuaiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("Peach") || card->inherits("Analeptic");
    }
};

class Qiuai: public TriggerSkill{
public:
    Qiuai():TriggerSkill("qiuai"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *buzhi, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.from == buzhi && !buzhi->isKongcheng()){
            Room *room = buzhi->getRoom();

            if(room->askForSkillInvoke(buzhi, objectName(), data)){
                const Card *showpeach = room->askForCard(buzhi, ".qiuai", "@qiuai");
                if(showpeach){
                    room->playSkillEffect("qiuai");
                    room->throwCard(showpeach);

                    LogMessage log;
                    log.type = "#Qiuai";
                    log.from = buzhi;
                    log.to << damage.to;
                    log.arg = QString::number(damage.damage);
                    log.arg2 = QString::number(damage.damage + 1);
                    room->sendLog(log);

                    damage.damage ++;
                    data = QVariant::fromValue(damage);
                }
            }
        }

        return false;
    }
};



JiguangCard::JiguangCard(){
    once = true;
}

bool JiguangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool JiguangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    switch(targets.length()){
    case 0: return true;
    case 1: {
            return !to_select->isKongcheng();
        }

    default:
        return false;
    }
}



void JiguangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *first = targets.first();
    ServerPlayer *second = targets.at(1);

    room->throwCard(this);
    room->showAllCards(second, first);

    LogMessage log;
    log.type = "#Jiguang";
    log.from = first;
    log.to << second;
    room->sendLog(log);
}

class Jiguang: public OneCardViewAsSkill{
public:
    Jiguang():OneCardViewAsSkill("jiguang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JiguangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JiguangCard *jiguang_card = new JiguangCard();
        jiguang_card->addSubcard(card_item->getFilteredCard());
        return jiguang_card;
    }
};


class Dongyue: public TriggerSkill{
public:
    Dongyue():TriggerSkill("dongyue"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *baibai, QVariant &data) const{
        int upper = qMin(5, baibai->getMaxHP());
        if(baibai->getHandcardNum() < upper){
            Room *room = baibai->getRoom();

            if(room->askForSkillInvoke(baibai, objectName(), data)){
                room->playSkillEffect("dongyue", qrand() % 2 + 1);
                int x = upper - baibai->getHandcardNum();
                if(x <= 0)
                    return false;
                baibai->drawCards(x);
            }
        }

        return false;
    }
};



class KunbangPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return !player->hasEquip(card) && card->isBlack();
    }
};




class Kunbang: public MasochismSkill{
public:
    Kunbang():MasochismSkill("kunbang"){
        events << DamageChainFinished;
    }


    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(player->isAlive() && event == DamageChainFinished)
            onDamaged(player, damage);

        return false;
    }

    virtual void onDamaged(ServerPlayer *wende, const DamageStruct &damage) const{

        if(wende->isKongcheng())
            return;

        Room *room = wende->getRoom();
        int x = damage.damage, i;

        for(i=0; i<x; i++){
            const Card *chaincard = room->askForCard(wende, ".kunbang", "@kunbang1");
            if(!chaincard)
                return;
            QList<ServerPlayer *> players = room->getAlivePlayers();
            ServerPlayer *toselect = room->askForPlayerChosen(wende, players, "@kunbang2");

            room->throwCard(chaincard);
            bool chained = ! toselect->isChained();
            room->setPlayerProperty(toselect, "chained", chained);
            //toselect->setChained(chained);
            room->broadcastProperty(toselect, "chained");
        }

    }
};


class Suoming: public TriggerSkill{
public:
    Suoming():TriggerSkill("suoming"){
        events << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.nature == DamageStruct::Normal && player->isChained()){
            LogMessage log;
            log.from = effect.from;
            log.type = "#SuomingProtect";
            log.to << player;
            room->sendLog(log);

            return true;
        }
        return false;
    }
};



class CannotJink: public SlashBuffSkill{
public:
    CannotJink():SlashBuffSkill("nj"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *player = effect.from;
        Room *room = player->getRoom();
        room->playSkillEffect("liegong");
        room->slashResult(effect, NULL);
        return true;

    }
};

JiecaoCard::JiecaoCard(){
    once = true;
}


bool JiecaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return to_select->getWeapon();
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return first->getWeapon() && first->canSlash(to_select);
    }else
        return false;
}


bool JiecaoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void JiecaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    if(!source->getMark("wucao")==0)
        source->drawCards(1, false);

    ServerPlayer *killer = targets.at(0);
    QList<ServerPlayer *> victims = targets;
    if(victims.length() > 1)
        victims.removeAt(0);
    const Weapon *weapon = killer->getWeapon();

    if(weapon == NULL)
        return;

    bool on_effect = room->cardEffect(this, source, killer);
    if(on_effect){
        QString prompt = QString("collateral-slash:%1:%2")
                         .arg(source->objectName()).arg(victims.first()->objectName());
        const Card *slash = room->askForCard(killer, "slash", prompt);
        if(slash){
            CardUseStruct use;
            use.card = slash;
            use.from = killer;
            use.to = victims;
            if(source->distanceTo(victims.first()) <= 1){
                room->acquireSkill(killer, "nj", false);
                room->useCard(use);
                killer->loseSkill("nj");
            }else{
                room->useCard(use);
            }

        }else{
            source->obtainCard(weapon);
        }
    }
    return;
}

class Jiecao: public OneCardViewAsSkill{
public:
    Jiecao():OneCardViewAsSkill("jiecao"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JiecaoCard");
    }


    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{

        JiecaoCard *jiecao_card = new JiecaoCard;
        jiecao_card->addSubcard(card_item->getCard()->getId());

        return jiecao_card;
    }
};


class Wucao: public PhaseChangeSkill{
public:
    Wucao():PhaseChangeSkill("wucao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("wucao") == 0
                && target->getPhase() == Player::Start
                && target->getAttackRange() >= 4;
    }

    virtual bool onPhaseChange(ServerPlayer *landao) const{
        Room *room = landao->getRoom();

        LogMessage log;
        log.type = "#WucaoWake";
        log.from = landao;
        room->sendLog(log);

        room->playSkillEffect("wucao");
        room->broadcastInvoke("animate", "lightbox:$wucao:5000");
        room->getThread()->delay(5000);

        room->setPlayerMark(landao, "wucao", 1);
        room->acquireSkill(landao, "kuanggu");

        room->loseMaxHp(landao);

        return false;
    }
};


class Gongzhu:public TriggerSkill{
public:
    Gongzhu():TriggerSkill("gongzhu"){
        frequency = Compulsory;
        events << CardEffect << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *buzki, QVariant &data) const{

        if(event == CardEffect){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->isNDTrick() && !effect.card->inherits("Collateral") && effect.from == buzki && effect.to->getGeneral()->isMale()){
                buzki->getRoom()->playSkillEffect("gongzhu", qrand() % 2 + 1);
                effect.card->onEffect(effect);
                return true;
            }

        }else if(event == CardUsed){
            CardUseStruct card_use = data.value<CardUseStruct>();
            CardStar card = card_use.card;
            if(card->inherits("Collateral") && card->getNumber()!=0 && card_use.from == buzki && card_use.to.at(0)->getGeneral()->isMale()){

                buzki->getRoom()->playSkillEffect("gongzhu", 3);
                buzki->getRoom()->throwCard(card);
                Collateral *collateral = new Collateral(Card::NoSuit, 0);
                collateral->setCancelable(false);
                card_use.from->playCardEffect(card);
                collateral->use(buzki->getRoom(), card_use.from, card_use.to);
                return true;
            }
        }

        return false;

    }
};


class Fenghuang:public TriggerSkill{
public:
    Fenghuang():TriggerSkill("fenghuang"){
        events << ChainStateChanged;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{

        Room *room = player->getRoom();
        ServerPlayer *buzki = room->findPlayerBySkillName(objectName());
        if(buzki && buzki->isAlive() && buzki->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect("fenghuang");
            buzki->drawCards(1,false);

        }
        return false;
    }
};


class Siji: public TriggerSkill{
public:
    Siji():TriggerSkill("siji"){
        events << Predamaged;
        frequency = Limited;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@crash") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *buzki, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = buzki->getRoom();
        if(damage.from != buzki && buzki->askForSkillInvoke(objectName(), data)){
            room->broadcastInvoke("animate", "lightbox:$siji");
            room->playSkillEffect(objectName());

            buzki->loseMark("@crash");
            if(damage.from){
                damage.to = damage.from;
                damage.chain = true;
                room->damage(damage);
                damage.to->turnOver();
            }
             return true;
        }

        return false;
    }
};


class Cipao: public TriggerSkill{
public:
    Cipao():TriggerSkill("cipao"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder){

            LogMessage log;
            log.type = "#CipaoProtect";
            log.to << player;
            log.from = damage.from;
            room->sendLog(log);

            return true;

        }

        return false;
    }
};


class Leiwu: public PhaseChangeSkill{
public:
    Leiwu():PhaseChangeSkill("leiwu"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("leiwu") == 0
                && target->getPhase() == Player::Discard
                && target->getHandcardNum() == 5;
    }

    virtual bool onPhaseChange(ServerPlayer *lfive) const{
        Room *room = lfive->getRoom();

        LogMessage log;
        log.type = "#LeiwuWake";
        log.from = lfive;
        room->sendLog(log);

        room->loseMaxHp(lfive);

        room->acquireSkill(lfive, "zhulei");
        room->acquireSkill(lfive, "wulei");
        room->setPlayerMark(lfive, "leiwu", 1);

        return false;
    }
};


class Zhulei: public OneCardViewAsSkill{
public:
    Zhulei():OneCardViewAsSkill("zhulei"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getSlashCount() == 0;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->isBlack() && to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        ThunderSlash *thunder_slash = new ThunderSlash(card->getSuit(), card->getNumber());
        thunder_slash->addSubcard(card->getId());
        thunder_slash->setSkillName(objectName());
        return thunder_slash;
    }
};

class Wulei: public TriggerSkill{
public:
    Wulei():TriggerSkill("wulei"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
            return false;

        if(!damage.from->hasSkill(objectName()))
            return false;

        ServerPlayer *lfive = damage.from;
        if(!damage.to->isChained() && damage.nature == DamageStruct::Thunder
           && lfive->askForSkillInvoke(objectName(), data))
        {
            Room *room = lfive->getRoom();

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = lfive;

            room->judge(judge);

            if(judge.isGood()){
                DamageStruct wulei_damage;
                wulei_damage.nature = DamageStruct::Thunder;
                wulei_damage.from = lfive;
                wulei_damage.to = player->getNextAlive();

                room->damage(wulei_damage);
            }
        }

        return false;
    }
};


class Hongmo: public PhaseChangeSkill{
public:
    Hongmo():PhaseChangeSkill("hongmo"){

    }

    virtual bool onPhaseChange(ServerPlayer *fouronezero) const{
        if(fouronezero->getPhase() != Player::Draw)
            return false;

        Room *room = fouronezero->getRoom();
        if(!fouronezero->askForSkillInvoke(objectName()))
            return false;

        room->playSkillEffect(objectName());

        QList<int> card_ids = room->getNCards(4);
        qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
        room->fillAG(card_ids);

        QString choice = room->askForChoice(fouronezero, "hongmo", "color+number");

        if(choice == "color"){
            foreach(int card_id, card_ids){
                const Card *card = Sanguosha->getCard(card_id);
                if(card->isRed())
                    room->takeAG(fouronezero, card_id);
                else
                    room->takeAG(NULL, card->getId());
            }
        }else{
            foreach(int card_id, card_ids){
                const Card *card = Sanguosha->getCard(card_id);
                if(card->getNumber()<=9)
                    room->takeAG(fouronezero, card_id);
                else
                    room->takeAG(NULL, card->getId());
            }
        }

        room->broadcastInvoke("clearAG");

        return true;
    }
};


ShenqiangCard::ShenqiangCard(){
}

bool ShenqiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select->isAllNude())
        return false;

    if(!targets.isEmpty())
        return false;

    return true;
}

void ShenqiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.from->getAttackRange() >= effect.to->getCards("hej").length()){
        foreach(const Card *cd, effect.to->getCards("hej")){
            room->moveCardTo(cd, effect.from, Player::Hand, false);
        }
    }else{

        int i;
        for(i = 0; i < effect.from->getAttackRange(); i++){
            int card_id = room->askForCardChosen(effect.from, effect.to, "hej", "shenqiang");
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), effect.from, Player::Hand, false);
            else
                room->obtainCard(effect.from, card_id);
        }

    }

    fouronezero->loseMark("@godspear");

}

class ShenqiangSelect: public ZeroCardViewAsSkill{
public:
    ShenqiangSelect():ZeroCardViewAsSkill("shenqiang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@godspear") > 0;
    }

    virtual const Card *viewAs() const{
        return new ShenqiangCard;
    }
};

class Shenqiang: public TriggerSkill{
public:
    Shenqiang():TriggerSkill("shenqiang"){
        events << PhaseChange;
        frequency = Limited;
        view_as_skill = new ShenqiangSelect;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasUsed("ShenqiangCard") && player->getMark("@godspear") == 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *fouronezero, QVariant &data) const{
        Room *room = fouronezero->getRoom();

        if(event == PhaseChange && fouronezero->getPhase() == Player::Finish){

            if(fouronezero->getCards("he").length() <= fouronezero->getAttackRange()){
                foreach(const Card *card, fouronezero->getCards("he")){
                    room->throwCard(card);
                }
            }
            else{
                room->askForDiscard(fouronezero, "shenqiang", fouronezero->getAttackRange(), false, true);
            }            
            room->loseMaxHp(fouronezero);
            room->acquireSkill(fouronezero, "wusheng");
        }

        return false;
    }
};



class Ximing: public PhaseChangeSkill{
public:
    Ximing():PhaseChangeSkill("ximing"){
    }

    virtual int getPriority() const{
        return 3; // very high priority
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        switch(target->getPhase()){
        case Player::Draw: {
                if(target->askForSkillInvoke("ximing", "draw")){
                    target->gainMark("@fate", 1);
                    return true;
                }
                break;
            }

        case Player::Play:{
                if(target->askForSkillInvoke("ximing", "play")){
                    target->gainMark("@fate", 1);
                    return true;
                }
                break;
            }

        default:
            return false;
        }

        return false;
    }
};


HuanmingCard::HuanmingCard(){
    target_fixed = true;
}

void HuanmingCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *maoding = card_use.from;

    QStringList choices;

    if(!maoding->hasFlag("huanming_draw")){
        choices << "draw";
    }

    if(!maoding->hasFlag("huanming_play")){
        choices << "play";
    }

    if(choices.isEmpty())
        return;

    QString choice;
    if(choices.length() == 1)
        choice = choices.first();
    else
        choice = room->askForChoice(maoding, "huanming", "draw+extra");

    maoding->loseMark("@fate");

    if(choice == "draw"){
        maoding->drawCards(2);
        room->setPlayerFlag(maoding, "huanming_draw");
    }else{
        room->setPlayerFlag(maoding, "huanming_play");
    }
}

class HuanmingViewAsSkill: public ZeroCardViewAsSkill{
public:
    HuanmingViewAsSkill():ZeroCardViewAsSkill("huanming"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !(player->hasFlag("huanming_draw") && player->hasFlag("huanming_play")) && player->getMark("@fate") > 0;
    }

    virtual const Card *viewAs() const{
        return new HuanmingCard;
    }
};

class Huanming: public PhaseChangeSkill{
public:
    Huanming():PhaseChangeSkill("huanming"){
        view_as_skill = new HuanmingViewAsSkill;
    }

    virtual int getPriority() const{
        return 4; // very high priority
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        switch(target->getPhase()){
        case Player::Judge: {
            if(target->getMark("@fate") > 0 && target->askForSkillInvoke("huanming", "judge")){
                target->loseMark("@fate", 1);
                return true;
            }
            break;
        }

        case Player::Discard:{
            if(target->getMark("@fate") > 0 && target->askForSkillInvoke("huanming", "discard")){
                target->loseMark("@fate", 1);
                return true;
            }
            break;
        }

        case Player::Finish: {
            Room *room = target->getRoom();
            if(target->hasFlag("huanming_draw")){
                room->setPlayerFlag(target, "-huanming_draw");
            }
            if(target->hasFlag("huanming_play")){
                room->setPlayerFlag(target, "-huanming_play");
            }

            break;
        }

        default:
            return false;
        }

        return false;
    }
};


QunyuanPackage::QunyuanPackage()
    :Package("qunyuan")
{
    General *jinteng;
    General *jim;
    General *huanying;
    General *onefourfour;
    General *buzhi;
    General *baibai;
    General *wende;
    General *landao;
    General *buzki;
    General *lfive;
    General *fouronezero;
    General *maoding;

    jinteng = new General(this, "jinteng", "wu", 4);
    jim = new General(this, "jim", "wu", 3);
    huanying = new General(this, "huanying", "shu", 3, false);
    onefourfour = new General(this, "onefourfour", "wu", 4);
    buzhi = new General(this, "buzhi", "wei", 4);
    baibai = new General(this, "baibai", "shu", 3, false);
    wende = new General(this, "wende", "wu", 4);
    landao = new General(this, "landao", "wu", 4);
    buzki = new General(this, "buzki", "shu", 3, false);
    lfive = new General(this, "lfive", "wei", 4);
    fouronezero = new General(this, "fouronezero", "wei", 4);
    maoding = new General(this, "maoding", "wei", 4);

    jinteng->addSkill(new EOS);

    jim->addSkill(new Zibao);
    jim->addSkill(new Juesi);

    huanying->addSkill(new Fuqin);
    huanying->addSkill(new Yiqi);
    patterns.insert(".yiqi", new YiqiPattern);

    onefourfour->addSkill(new Zhongguo);
    onefourfour->addSkill(new Chongqi);
    patterns.insert(".chongqi", new ChongqiPattern);

    buzhi->addSkill(new Qiuai);
    patterns.insert(".qiuai", new QiuaiPattern);

    baibai->addSkill(new Jiguang);
    baibai->addSkill(new Dongyue);

    wende->addSkill(new Kunbang);
    wende->addSkill(new Suoming);
    patterns.insert(".kunbang", new KunbangPattern);

    landao->addSkill(new Jiecao);
    landao->addSkill(new Wucao);
    related_skills.insertMulti("wucao", "kuanggu");

    buzki->addSkill(new Gongzhu);
    buzki->addSkill(new Fenghuang);
    buzki->addSkill(new MarkAssignSkill("@crash", 1));
    buzki->addSkill(new Siji);

    related_skills.insertMulti("siji", "#@crash-1");

    lfive->addSkill(new Cipao);
    lfive->addSkill(new Leiwu);

    fouronezero->addSkill(new Hongmo);
    fouronezero->addSkill(new MarkAssignSkill("@godspear", 1));
    fouronezero->addSkill(new Shenqiang);

    //related_skills.insertMulti("shenqiang", "wusheng");
    related_skills.insertMulti("shenqiang", "#@godspear-1");

    maoding->addSkill(new Ximing);
    maoding->addSkill(new MarkAssignSkill("@fate", 1));
    maoding->addSkill(new Huanming);


    addMetaObject<EOSCard>();
    addMetaObject<ZibaoCard>();
    addMetaObject<YiqiCard>();
    addMetaObject<ChongqiCard>();
    addMetaObject<JiguangCard>();
    addMetaObject<JiecaoCard>();
    addMetaObject<ShenqiangCard>();
    addMetaObject<HuanmingCard>();

    skills << new CannotJink << new Zhulei << new Wulei;
}

ADD_PACKAGE(Qunyuan);
