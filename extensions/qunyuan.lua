--头文件
module("extensions.qunyuan",package.seeall)
extension=sgs.Package("qunyuan")
--武将定义
jinteng=sgs.General(extension,"jinteng","wu",4,true)--近藤
huanying=sgs.General(extension,"huanying","shu",3,false)--幻樱
onefourfour=sgs.General(extension,"onefourfour","wu",4,true)--144
buzhi=sgs.General(extension,"buzhi","wei",4,true)--不知
--WenDe=sgs.General(extension,"WenDe","wu",4,true)--文德
--LanDao=sgs.General(extension,"LanDao","wu",4,true)--蓝刀
--技能定义
----EOS
EOSCard=sgs.CreateSkillCard{
	name="EOSCard",
	once = true,
	will_throw = false,
	
	
	filter = function(self, targets, to_select)
        if(#targets > 1) then return false end
        
        if(to_select == self) then return false end
        
		return to_select:getArmor() or to_select:getDefensiveHorse() or to_select:getOffensiveHorse()
    end,

	
	on_effect = function(self, effect)
		local room = effect.from:getRoom()
		options = ""
		if(effect.to:getArmor())then
			options = options.."fangju+"
		end
		if(effect.to:getDefensiveHorse())then
			options = options.."fangyuma+"
		end
		if(effect.to:getOffensiveHorse())then
			options = options.."jingongma+"
		end
		
		local choice = room:askForChoice(effect.from, "EOS", string.sub(options, 1, string.len(options)-1))


		if(choice == "fangju")then
			if(effect.to:getArmor())then
				room:moveCardTo(self, effect.to, sgs.Player_Hand, false)
				room:moveCardTo(effect.to:getEquip(1), effect.from, sgs.Player_Hand)
			end
		elseif(choice == "fangyuma")then
			if(effect.to:getDefensiveHorse())then
				room:moveCardTo(self, effect.to, sgs.Player_Hand, false)
				room:moveCardTo(effect.to:getEquip(2), effect.from, sgs.Player_Hand)
			end
		else
			if(effect.to:getOffensiveHorse())then
				room:moveCardTo(self, effect.to, sgs.Player_Hand, false)
				room:moveCardTo(effect.to:getEquip(3), effect.from, sgs.Player_Hand)
			end
		end
		room:setPlayerFlag(effect.from, "EOS-used")
		--room:playSkillEffect("EOS")
	end
}
EOS=sgs.CreateViewAsSkill{
	name="EOS",
	n=1,
	enabled_at_play = function()
        return not sgs.Self:hasFlag("EOS-used")
    end,
	view_filter = function(self, selected, to_select)
        return to_select:inherits("EquipCard")
		--return true
    end,
	view_as=function(self, cards)
        local acard=EOSCard:clone() 
        if #cards==1 then
            acard:addSubcard(cards[1])
			acard:setSkillName(self:objectName())
			return acard
        end
	end
}

--抚琴
fuqin = sgs.CreateTriggerSkill{
	name = "fuqin",
    events = {sgs.PhaseChange, sgs.CardLost},
    frequency = sgs.Skill_Frequent,
	on_trigger = function(self, event, player, data)
        if(event == sgs.CardLost) then
			local move = data:toCardMove()
			if(move.to_place == sgs.Player_DiscardedPile)then
                player:gainMark("fuqin", 1)
                if(player:getMark("fuqin") == 2 and player:isWounded())then
                    if(player:askForSkillInvoke("fuqin"))then
                        local recov = sgs.RecoverStruct()
						recov.recover = 1
						recov.who = player
						player:getRoom():recover(player, recov)
                    end
                end
            end
        elseif(event == sgs.PhaseChange) then
			player:loseMark("fuqin", player:getMark("fuqin"))
        end
    end
}

--弈棋
yiqiCard=sgs.CreateSkillCard{
	name="yiqi",
	once = true,
	
	
	filter = function(self, targets, to_select)
        if(#targets > 1) then return false end
        
        if(to_select == self) then return false end
        
		return not to_select:isKongcheng()
    end,

	
	on_effect = function(self, effect)
		local room = effect.from:getRoom()
		local show_card = room:askForCardShow(effect.to, effect.from, "yiqi")
		
		room:showCard(effect.to, show_card:getEffectiveId())
		if(show_card:inherits("TrickCard"))then
			local give_card = room:askForCard(effect.from, "TrickCard", "@yiqi")
			if(give_card~=nil) then
				effect.to:obtainCard(give_card)
			else
				effect.to:drawCards(1)
			end
		else
			effect.from:obtainCard(show_card)
		end
		room:setPlayerFlag(effect.from, "yiqi-used")
	end
}
yiqi=sgs.CreateViewAsSkill{
	name="yiqi",
	n=0,
	enabled_at_play = function()
        return not sgs.Self:hasFlag("yiqi-used")
    end,
	view_as=function(self, cards)
        local card=yiqiCard:clone()
		card:setSkillName(self:objectName())
		return card
	end
}
--中国
zhongguo = sgs.CreateTriggerSkill{
	name = "zhongguo",
	priority = 2,
    events = {sgs.SlashMissed},
	
	on_trigger = function(self, event, player, data)
		local room = player:getRoom()
		local effect = data:toSlashEffect()
        if(not player:isNude() and not player:getWeapon()) then
			if(player:askForSkillInvoke("zhongguo"))then
                local card = room:askForCard(player, ".|.|.|.", "@zhongguo")
				room:throwCard(card)
				room:slashResult(effect, nil)
            end
        end
		return false
    end
}
--重启
chongqi_card = sgs.CreateSkillCard{
    name = "chongqi",
    target_fixed = true,
    will_throw = true,
    
    on_use = function(self, room, source, targets)
        if(source:isAlive()) then
            room:drawCards(source, 1)
            room:throwCard(self)
        end
    end
}

chongqi = sgs.CreateViewAsSkill{
	name = "chongqi",
	n = 1,
	
    enabled_at_play = function()
        return true
    end,
	
	view_filter = function(self, selected, to_select)
        return to_select:inherits("EquipCard")
    end,
	
	view_as = function(self, cards)
        if #cards > 0 then
            local new_card = chongqi_card:clone()
            local card = cards[1]
            new_card:addSubcard(card:getId())
            new_card:setSkillName("chongqi")
            return new_card
        else return nil
        end
    end
}

--求爱
qiuaiCard = sgs.CreateSkillCard{
	name = "qiuaiCard",
	target_fixed = true,
	will_throw = true,
	filter = function(self, targets, to_select)
		return true
	end,
	on_effect = function(self, effect)
	end
}
qiuaiVS = sgs.CreateViewAsSkill{
	name = "qiuai", 
	n = 1, 
	view_filter = function(self, selected, to_select)
		return to_select:inherits("Peach") or to_select:inherits("Analeptic")
	end, 
	view_as = function(self, cards) 
		if #cards == 1 then
			local card = qiuaiCard:clone()
			card:addSubcard(cards[1])
			return card
		end
	end, 
	enabled_at_play = function(self, player)
		return true
	end, 
	enabled_at_response = function(self, player, pattern)
		return pattern == "@@qiuai"
	end
}
qiuai = sgs.CreateTriggerSkill{
	name = "qiuai",
    events = {sgs.Predamage},
	view_as_skill = qiuaiVS, 
	
	on_trigger = function(self, event, player, data)
		local damage = data:toDamage()
		local room = player:getRoom()
		if( damage.card:inherits("Slash") and damage.from:objectName() == player:objectName() and not player:isKongcheng())then
			if(room:askForSkillInvoke(player, "qiuai", data)) then
				local card = room:askForCard(player, "@@qiuai", "@qiuai")
				if(card~=nil)then
					room:throwCard(card)
					damage.damage = damage.damage + 1
					data:setValue(damage)
				end
			end
        end
		return false
    end
}
--添加技能
jinteng:addSkill(EOS)
huanying:addSkill(fuqin)
huanying:addSkill(yiqi)
onefourfour:addSkill(zhongguo)
onefourfour:addSkill(chongqi)
buzhi:addSkill(qiuai)
--翻译
sgs.LoadTranslationTable{
	["qunyuan"]="群员",
	--
	["jinteng"] = "近藤",
	["jim"] = "基姆",
	["huanying"] = "幻樱",
	["onefourfour"] = "144",
	["buzhi"] = "不知",
	["baibai"] = "白白",
	["wende"] = "文德",
	["landao"] = "蓝刀",
	["buzki"] = "布鸶姬",
	["lfive"] = "乱舞",
	["fouronezero"] = "410",
	["maoding"] = "猫町",
	
	["zibao"] = "自爆",
	["juesi"] = "决死",
	["fuqin"] = "抚琴",
	["yiqi"] = "弈棋",
	["zhongguo"] = "中国",
	["chongqi"] = "重启",
	["qiuai"] = "求爱",
	["jiguang"] = "激光",
	["dongyue"] = "冬月",
	["kunbang"] = "捆绑",
	["suoming"] = "锁命",
	["jiecao"] = "节操",
	["wucao"] = "无操",
	["gongzhu"] = "公主",
	["fenghuang"] = "凤凰",
	["siji"] = "死机",
	["cipao"] = "磁炮",
	["leiwu"] = "雷舞",
	["zhulei"] = "注雷",
	["wulei"] = "舞雷",
	["hongmo"] = "红魔",
	["shenqiang"] = "神枪",
	["ximing"] = "惜命",
	["huanming"] = "换命",
	
	[":EOS"] = "出牌阶段你可以指定场上任何一名已装备防具牌或1马的其他角色，并将你手牌或装备区里的一张装备牌交给该角色，然后你获得其防具牌或1马",
	[":zibao"] = "出牌阶段，你可以减少1点体力上限，然後令与你距离2以内的一名其他角色失去1点体力。每回合限用一次",
	[":juesi"] = "当你处于濒死状态时，你可以展示牌堆顶的X张牌，X为你当前的体力上限且最多为4。你可以选择弃置其中两张同色的牌并增加一点体力上限，若如此做可再选择弃置两张同色的牌并回复一点体力，然后获得其余的牌",
	[":fuqin"] = "弃牌阶段若你弃置两张或以上的手牌，你可以回复一点体力",
	[":yiqi"] = "出牌阶段，你可以指定除你以外任一有手牌的角色，该角色需展示一张手牌，若该牌为锦囊牌，你须交给对方一张锦囊牌或让其摸一张牌；若不为锦囊牌，你立即获得这张牌。每回合限用一次",
	[":zhongguo"] = "如果你没有装备武器，当你的【杀】被【闪】抵消时，你可以弃一张牌并对目标强制造成伤害",
	[":chongqi"] = "出牌阶段，你可以丢弃你的一张装备牌，然后从牌堆摸一张牌",
	[":qiuai"] = "出牌阶段，你使用的【杀】造成伤害时你可以弃置一张【桃】或【酒】，若如此作则你造成的伤害+1",
	[":jiguang"] = "出牌阶段，你可以弃一张手牌，并先后指定两名角色。第一名角色可观看一次第二名角色的手牌，每回合限一次",
	[":dongyue"] = "每当你受到一次伤害时，可以先将你手牌数补到体力上限的张数，然后进行伤害的结算",
	[":kunbang"] = "每当你受到1点伤害，你可以立即打出一张黑色手牌。若如此做，视为你对一名角色使用了一张【铁索连环】",
	[":suoming"] = "<b>锁定技</b>，当你处于连环状态时，无属性【杀】对你造成的伤害无效",
	[":jiecao"] = "出牌阶段，你可以弃一张梅花手牌并指定装备区里有武器牌的一名角色。该角色需对其攻击范围内，你指定的另一名角色使用一张【杀】，否则将武器交给你。若另一名角色与你距离为1以内，此【杀】不可被闪避。每回合限用一次",
	[":wucao"] = "<b>觉醒技</b>，回合开始阶段，若你的攻击距离不小于4，则你须减一点体力上限并永久获得技能“狂骨”。每当你使用“节操”技能，你可以立即摸一张牌",
	[":gongzhu"] = "<b>锁定技</b>，你对其他男性角色使用的非延时类锦囊不可以被【无懈可击】抵消",
	[":fenghuang"] = "每当有一名角色被横置或重置时，你可以立即摸一张牌",
	[":siji"] = "<b>限定技</b>，当你受到一次伤害时，你可以转移此伤害给伤害的来源，然后该角色将其武将牌翻面",
	[":cipao"] = "<b>锁定技</b>，你使用【雷杀】时无距离限制。雷属性的伤害对你无效",
	[":leiwu"] = "<b>觉醒技</b>，出牌阶段结束时，若你的手牌数量为5，你须减一点体力上限，并永久获得技能“注雷”和“舞雷”",
	[":zhulei"] = "你可以将你的黑色的【杀】当作【雷杀】来使用",
	[":wulei"] = "你对一名不处于连环状态的玩家造成雷属性伤害时可以进行一次判定，若结果为黑色，你对造成伤害的角色下家造成一点雷属性伤害",
	[":hongmo"] = "摸牌阶段，你可以选择执行以下行动来取代摸牌：从牌堆顶亮出四张牌，拿走其中所有红色的牌或者所有点数不大于9的牌，然后弃掉其余的牌",
	[":shenqiang"] = "<b>限定技</b>，出牌阶段，你可以从一名其他角色的区域获得等同于你攻击范围数量的牌(不足则全部获得)，若如此做，回合结束阶段开始时，你须弃置等同于你攻击范围数量的牌（不足则全弃），然后减1点体力上限，并获得技能“武圣”",
	[":ximing"] = "你可以选择以下一至两项：1.跳过你该回合的摸牌阶段 2.跳过你该回合的出牌阶段。你每做出上述之一选择，你获得1枚“宿命”标记。游戏开始时，你获得1枚“宿命”标记。",
	[":huanming"] = "你可以选择以下一至四项：1.跳过你该回合的判定阶段  2. 在出牌阶段摸两张牌 3. 出牌阶段你可以额外使用一张【杀】和【酒]】 4.跳过你该回合的弃牌阶段。你每做出上述之一选择，你必须先弃置1枚“宿命”标记，上述每项每回合限一次。",

	
	["EOS:fangju"] = "获得对方已装备的防具",
	["EOS:fangyuma"] = "获得对方已装备的+1马",
	["EOS:jingongma"] = "获得对方已装备的-1马",
	["hongmo:color"] = "获得其中所有红色的牌",
	["hongmo:number"] = "获得其中所有点数不大于9的牌",
	["juesi:maxhp"] = "弃置两张同色的牌并增加一点体力上限",
	["juesi:hp"] = "弃置两张同色的牌并回复一点体力",
	["juesi:cancel"] = "取消",
	["ximing:draw"] = "您是否想跳过摸牌阶段？若如此做可获得一枚“宿命”标记",
	["ximing:play"] = "您是否想跳过出牌阶段？若如此做可获得一枚“宿命”标记",
	["huanming:judge"] = "您是否想弃掉一枚“宿命”标记，并跳过判定阶段？",
	["huanming:draw"] = "摸两张牌",
	["huanming:extra"] = "可以额外使用一次【酒】和【杀】",
	["huanming:discard"] = "您是否想弃掉一枚“宿命”标记，并跳过弃牌阶段？",
	
	["@yiqi"] = "请将一张锦囊牌交给对方，若不如此做对方摸一张牌",
	["@zhongguo"] = "是否丢弃一张牌使该杀伤害生效？",
	["@chongqi"] = "请选择一张装备牌重铸",
	["@qiuai"] = "请弃置一张【桃】或【酒】",
	["@kunbang1"] = "你可以选择弃一张黑色手牌，视为你对一名角色使用了一张【铁索连环】",
	["@kunbang2"] = "请选择一名角色使用【铁索连环】",
	["@crash"] = "死机",
	["@godspear"] = "神枪",
	["@fate"] = "宿命",
	
	["#Qiuai"] = "%from 的技能求爱生效，对 %to 的伤害从 %arg 增加到 %arg2",
	["#Jiguang"] = "白白使用技能【激光】，指定 %from 观看了 %to 的手牌",
	["#ArmorNullify"] = "%from 的防具【%arg】技能被触发，【%arg2】对其无效",
	["#SuomingProtect"] = "%to 的【锁命】锁定技被触发，(%from)的无属性杀伤害无效",
	["#CipaoProtect"] = "%to 的【磁炮】锁定技被触发，(%from)的雷电属性伤害无效",
	
	["#WucaoWake"] = "%from 攻击距离不小于4，触发了觉醒技【无操】",
	["#LeiwuWake"] = "%from 的手牌数量等于5，觉醒技【雷舞】触发",
	
	["$EOS"] = "EOS Shielding System Activated",
	["$zibao"] = "噗噗子，快使出自爆！",
	["$juesi"] = "（Miss音）",
	["$jiguang1"] = "白白激光，Shoooto！～",
	["$jiguang2"] = "你的内部已经被我看穿啦！",
	["$dongyue1"] = "等等，这个炮台，这个零件，怎么没见过……呜……",
	["$dongyue2"] = "唉？唉？坑爹啊！",
	["$qiuai"] = "冰天雪地三百六十度旋转求……爱",
	["$jiecao"] = "金克拉有什么好处？说对了就给他",
	["$wucao"] = "节操是什么能吃吗，晒死你们哈哈哈哈哈",
	["$gongzhu1"] = "狗尼萨马~~",
	["$gongzhu2"] = "文德公……想死几次呢？",
	["$gongzhu3"] = "零度……好人……白白……zzz 师太！",
	["$fenghuang"] = "人事部的布斯基来报道了",
	["$siji"] = "哼，区区在之，能奈我何",
	["cv:jinteng"] = "零度",
	["cv:jim"] = "零度",
	["cv:baibai"] = "笑笑",
	["cv:buzhi"] = "零度",
	["cv:landao"] = "零度",
	["cv:buzki"] = "笑笑",
}

