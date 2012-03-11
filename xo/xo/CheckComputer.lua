function IO( Field )
	local j = 0
	for i = 1, 3 do		
		--горизонталь
		if Field[i+j] == 0 and Field[i+j+2] == 0 and Field[i+j+1] == 10 then
			return 1,i-1,100
		end
		if Field[i+j] == 0 and Field[i+j+1] == 0 and Field[i+j+2] == 10 then
			return 2,i-1,100
		end
		if Field[i+j+1] == 0 and Field[i+j+2] == 0 and Field[i+j] == 10 then
			return 0,i-1,100
		end	
		--вертикаль	
		if Field[i] == 0 and Field[i+6] == 0 and Field[i+3] == 10 then
			return i-1,1,100
		end
		if Field[i] == 0 and Field[i+3] == 0 and Field[i+6] == 10 then
			return i-1,2,100
		end
		if Field[i+3] == 0 and Field[i+6] == 0 and Field[i] == 10 then
			return i-1,0,100
		end			
		j = j + 2
	end
	--диагональ
	if Field[1] == 0 and Field[5] == 0 and Field[9] == 10 then
		return 2,2,100
	end
	if Field[1] == 0 and Field[9] == 0 and Field[5] == 10 then
		return 1,1,100
	end
	if Field[5] == 0 and Field[9] == 0 and Field[1] == 10 then
		return 0,0,100
	end
	--диагональ
	if Field[3] == 0 and Field[5] == 0 and Field[7] == 10 then
		return 0,2,100
	end
	if Field[3] == 0 and Field[7] == 0 and Field[5] == 10 then
		return 1,1,100
	end
	if Field[5] == 0 and Field[7] == 0 and Field[3] == 10 then
		return 2,0,100
	end
	--закрыть позицию
	local j = 0
	for i = 1, 3 do		
		--горизонталь
		if Field[i+j] == 1 and Field[i+j+2] == 1 and Field[i+j+1] == 10 then
			return 1,i-1,200
		end
		if Field[i+j] == 1 and Field[i+j+1] == 1 and Field[i+j+2] == 10 then
			return 2,i-1,200
		end
		if Field[i+j+1] == 1 and Field[i+j+2] == 1 and Field[i+j] == 10 then
			return 0,i-1,200
		end	
		--вертикаль	
		if Field[i] == 1 and Field[i+6] == 1 and Field[i+3] == 10 then
			return i-1,1,200
		end
		if Field[i] == 1 and Field[i+3] == 1 and Field[i+6] == 10 then
			return i-1,2,200
		end
		if Field[i+3] == 1 and Field[i+6] == 1 and Field[i] == 10 then
			return i-1,0,200
		end			
		j = j + 2
	end
	--диагональ
	if Field[1] == 1 and Field[5] == 1 and Field[9] == 10 then
		return 2,2,200
	end
	if Field[1] == 1 and Field[9] == 1 and Field[5] == 10 then
		return 1,1,200
	end
	if Field[5] == 1 and Field[9] == 1 and Field[1] == 10 then
		return 0,0,200
	end
	--диагональ
	if Field[3] == 1 and Field[5] == 1 and Field[7] == 10 then
		return 0,2,200
	end
	if Field[3] == 1 and Field[7] == 1 and Field[5] == 10 then
		return 1,1,200
	end
	if Field[5] == 1 and Field[7] == 1 and Field[3] == 10 then
		return 2,0,200
	end
	if Field[5] == 10 then
		return 1,1,300
	end
	if Field[9] == 10 then
		return 2,2,300
	end
	--
	math.randomseed(os.time())
	math.random()
	local i = 1
	repeat	
        x = math.random(0,8) + 1 	
		if Field[x] == 10 then
			return (x-1) % 3, (x-1) / 3, x
		end
		i = i + 1
	until i > 1000
	return -1,-1, x
end

function Out()
	
	return math.random(0,8) + 1  
end
