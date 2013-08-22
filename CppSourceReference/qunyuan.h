#ifndef QUNYUAN_H
#define QUNYUAN_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"


class EOSCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE EOSCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZibaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZibaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YiqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YiqiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ChongqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChongqiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class JiguangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiguangCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JiecaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiecaoCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShenqiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenqiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuanmingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanmingCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};


class QunyuanPackage : public Package{
    Q_OBJECT

public:
    QunyuanPackage();
};

#endif // QUNYUAN_H
