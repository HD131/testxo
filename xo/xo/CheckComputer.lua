Field = {}

function IO()
    Field = SaveField()	
	local x,y
	for i = 1, 3 do
		local j = 0;
		if Field[i+j] == 1 and Field[i+j+2] == 1 then
			x = 1
			y = i - 1
			break
		end
		if Field[i+j] == 1 and Field[i+j+1] == 1 then
			x = 2
			y = i - 1
			break
		end
		if Field[i+j+1] == 1 and Field[i+j+2] == 1 then
			x = 0
			y = i - 1
			break
		end
		j = j + 2
	end
	if Field[9] == 10 then
		x = 2
		y = 2
	end
	a,b = x,y
	return x,y	
end