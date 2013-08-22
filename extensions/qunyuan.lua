--头文件
module("extensions.qunyuan",package.seeall)
extension=sgs.Package("qunyuan")
--武将定义
JinTeng=sgs.General(extension,"JinTeng","wu",4,true)--近藤
--HuanYing=sgs.General(extension,"HuanYing","shu",3,false)--幻樱
--OneFourFour=sgs.General(extension,"OneFourFour$","wu",4,true)--144
--BuZhi=sgs.General(extension,"BuZhi","wei",4,true)--不知
--WenDe=sgs.General(extension,"WenDe","wu",4,true)--文德
--LanDao=sgs.General(extension,"LanDao","wu",4,true)--蓝刀
--技能定义
----赌书
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
				room:moveCardTo(self, effect.to, sgs.Player_PlaceHand, false)
				room:moveCardTo(effect.to:getEquip(1), effect.from, sgs.Player_PlaceHand)
			end
		elseif(choice == "fangyuma")then
			if(effect.to:getDefensiveHorse())then
				room:moveCardTo(self, effect.to, sgs.Player_PlaceHand, false)
				room:moveCardTo(effect.to:getEquip(2), effect.from, sgs.Player_PlaceHand)
			end
		else
			if(effect.to:getOffensiveHorse())then
				room:moveCardTo(self, effect.to, sgs.Player_PlaceHand, false)
				room:moveCardTo(effect.to:getEquip(3), effect.from, sgs.Player_PlaceHand)
			end
		end
		room:setPlayerFlag(effect.from, "EOS-used")
		--room:playSkillEffect("EOS")
		
		local log = sgs.LogMessage()
		log.type = "#luadimeng"
		log.from = a
		log.to:append(b)
		log.arg  = n1
		log.arg2 = n2
		room:sendLog(log)
	end
}
EOS=sgs.CreateViewAsSkill{
	name="EOS",
	n=1,
	enabled_at_play = function()
        return not sgs.Self:hasFlag("EOS-used")
    end,
	view_filter = function(self, selected, to_select)
        return to_select:isKindOf("EquipCard")
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

--添加技能
JinTeng:addSkill(EOS)
--翻译
sgs.LoadTranslationTable{
	["qunyuan"]="群员",
	--
	["JinTeng"]="近藤",
	["#JinTeng"]="多大优等生",
	["designer:JinTeng"]="沙发",
	["EOS"]="EOS",
	[":EOS"] = "出牌阶段你可以指定场上任何一名已装备防具牌或1马的其他角色，并将你手牌或装备区里的一张装备牌交给该角色，然后你获得其防具牌或1马",
	["EOS:fangju"] = "获得对方已装备的防具",
	["EOS:fangyuma"] = "获得对方已装备的+1马",
	["EOS:jingongma"] = "获得对方已装备的-1马",
	["$EOS"] = "EOS Shielding System Activated",
	["cv:JinTeng"] = "零度",
}

