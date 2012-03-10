function IO( Field )
	for i = 1, 3 do
		local j = 0
		--local i = 1
		if Field[i+j] == 1 and Field[i+j+2] == 1 and Field[i+j+1] == 10 then
			return 1,i-1
		end
		if Field[i+j] == 1 and Field[i+j+1] == 1 and Field[i+j+2] == 10 then
			return 2,i-1
		end
		if Field[i+j+1] == 1 and Field[i+j+2] == 1 and Field[i+j] == 10 then
			return 0,i-1
		end		
		j = j + 2
	end
	if Field[9] == 10 then
		return 2,2
	end
	return -1,-1
end

