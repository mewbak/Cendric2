onWin = function(W)
	W:addConditionProgress("boss", "BossMercenaries")
	W:setMap("res/map/gandriatavern/gandriatavern.tmx", 916, 465)
end
	
onLose = function(W)
	W:setMap("res/map/gandriatavern/gandriatavern.tmx", 1016, 280)
end