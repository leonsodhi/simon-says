symbol startBtn = pinC.3
symbol countPin = C.2
symbol ledMSB = C.4
symbol ledLSB = C.0
symbol buttonsPin = C.1

symbol seg7Count = b1
symbol btnHit = b2
symbol numSteps = b4

symbol STEP_DATA_START = 28
symbol BTN1_HIT = 1
symbol BTN2_HIT = 2
symbol BTN3_HIT = 3



start0:
	input C.3
	output countPin
	output ledMSB
	output ledLSB
	input buttonsPin
	
	low countPin
	
	numSteps = 0
	call initSteps
	
	call runDemo
	seg7Count = 0	;7seg is reset in hw
	
	goto main
	
main:	
	call ledsoff	
	call incCount		;start counter @ 1
	pause 1000
	
	symbol i = b5
	symbol k = b6
	symbol stepCount = b7
	symbol thisStepStart = b8

	thisStepStart = STEP_DATA_START
	for i = 1 to numSteps
		bptr = thisStepStart
		stepCount = @bptrinc
		
		;======================
		for k = 1 to stepCount
			call bptrLedOn
			pause 1000
			call ledsoff
			pause 500
			inc bptr
		next k
		;======================

		;======================		
		bptr = thisStepStart
		stepCount = @bptrinc
		for k = 1 to stepCount
			call readButtonsLoop			
			call bptrLedOn
			pause 100
			if btnHit != @bptrinc then
				 ;reset 7seg to 0
				k = 10 - seg7Count
				for i = 1 to k
					call incCount
				next i
				goto start0
			else
				call ledsoff				
			end if
		next k
		;======================
		
		thisStepStart = bptr
		call incCount
	next i
	
	goto missionComplete


initSteps:	
	bptr = STEP_DATA_START
	
	@bptrinc = 3	;count
	@bptrinc = 1
	@bptrinc = 2
	@bptrinc = 3
	inc numSteps
	
	@bptrinc = 5	;count
	@bptrinc = 2
	@bptrinc = 1
	@bptrinc = 2
	@bptrinc = 3
	@bptrinc = 1
	inc numSteps
	
	;this isn't very random!
	symbol randomByte1 = b10
	symbol randomByte2 = b11
	symbol randomWord = w5
	@bptrinc = 5	;count
	randomWord = time
	for i = 1 to 5
		random randomWord
		@bptrinc = b10 & 3
	next i
	inc numSteps

	;Counter imposes a limit of 9 steps	
	numSteps = numSteps MAX 9
	
	return

	
runDemo:
	call led1On
	for i = 0 to 6
		if startBtn = 1 then
			call waitForStartButt
			return
		else
			pause 50
		end if
	next i
			
	call led2On
	for i = 0 to 6
		if startBtn = 1 then
			call waitForStartButt
			return
		else
			pause 50
		end if
	next i
	
	call led3On
	for i = 0 to 6
		if startBtn = 1 then
			call waitForStartButt
			return
		else
			pause 50
		end if
	next i
	
	goto runDemo


waitForStartButt:
	;wait utill button isn't pressed
	if startBtn = 1 then
		goto waitForStartButt
	end if
	return


missionComplete:
	call incCount
	if startBtn = 1 then
		call waitForStartButt
		goto start0
	end if
	goto missionComplete


bptrLedOn:
	select case @bptr
		case 1
			call led1On
		case 2
			call led2On
		case 3
			call led3On
		else
	endselect		
	return
	

led1On:
	low ledMSB
	high ledLSB
	return

led2On:
	high ledMSB
	low ledLSB
	return

led3On:
	high ledMSB
	high ledLSB
	return
	
ledsOff:
	low ledMSB
	low ledLSB
	return
	

incCount:
	pulsout countPin,10
		
	inc seg7Count
	seg7Count = seg7Count % 10
	return
	

readButtonsLoop:
	symbol ledResist = b0
	disconnect
	readadc buttonsPin, ledResist
	pause 100
	if ledResist >= 150 AND ledResist <= 160 then
		btnHit = BTN1_HIT
	elseif ledResist >= 170 AND ledResist <= 180 then
		btnHit = BTN2_HIT
	elseif ledResist >= 190 AND ledResist <= 200 then
		btnHit = BTN3_HIT
	else 
		goto readButtonsLoop
	endif
	return
	