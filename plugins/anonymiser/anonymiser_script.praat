#praat script: anonymise_long_sound.praat

#version: [2013-03-06]

#author: Daniel Hirst
#email: daniel.hirst@lpl-aix.fr

#purpose: replace portions of a long sound which are labelled with a key word on the accompanying TextGrid
#		with a hum sound with the same prosodic characteristics as the original sound
#		Original long sound can be mono or stereo, anonymised sound will be same.

#requires: the folder containing the Long_Sounds to be anonymised may be specified or selected with the browser
#		each sound should be accompanied by a TextGrid with the same name

form anonymise_long_sound
	comment Leave empty to select with browser:
	sentence Folder 
	word sound_extension .wav
	word textGrid_extension .TextGrid
	word anonymised_extension _anon.wav
	optionmenu Output_format: 1
		option WAV
		option AIFF
		option AIFC
		option Next/Sun
		option NIST
	natural target_tier 1
	word target_label buzz
	comment duration of section for analysis (in secs)
	positive section 30
	positive timestep 0.01
	boolean automatic_max_and_min yes 
	natural minimum_f0 60
	natural maximum_f0 700
	comment use this to lower overall intensity if necessary
	positive scale_intensity 0.9
	comment Feedback on progress:
	boolean verbose yes
endform

precision = 50

if verbose
	clearinfo
endif

if folder$ = ""
	folder$ = chooseDirectory$("Select the folder containing the Sounds and TextGrids to anonymise")
endif

mySounds = Create Strings as file list... Sounds 'folder$'/*'sound_extension$'
nSounds = Get number of strings

for iSound to nSounds
	select mySounds
	sound$ = Get string... iSound

	if not endsWith(sound$, anonymised_extension$)
		name$ = sound$ - sound_extension$
		textGrid$ = name$ + textGrid_extension$
		anonymised_sound$ = name$ + anonymised_extension$

		if not fileReadable(folder$+"/"+textGrid$)
			printline Can't find TextGrid file for 'name$'
		else
			call treat_sound
		endif
	endif
endfor

select mySounds
Remove



procedure treat_sound
	if verbose
		printline Treating file 'sound$'
	endif
	mySound = Open long sound file... 'folder$'/'sound$'
	sound_duration = Get total duration
	sampling_frequency = Get sampling frequency
	myTextGrid = Read from file... 'folder$'/'textGrid$'
	select mySound
	part_end = 0
	iPart = 0

	repeat
		iPart = iPart+1
		part_start = part_end
		part_end = part_end + section
	
		if part_end > sound_duration
			part_end = sound_duration
		endif

		select mySound
		call treat_part
	until part_end = sound_duration

	myNew_sound = Open long sound file... 'folder$'/'anonymised_sound$'
	pause - Click to continue
	select mySound
	plus myNew_sound
	plus myTextGrid
	Remove
endproc

procedure treat_part
	nTargets = 0
	mySound_part = Extract part... part_start part_end no
	nChannels = Get number of channels
	intensity = Get intensity (dB)
	scaled_intensity = intensity * scale_intensity
	Scale intensity... scaled_intensity


	select myTextGrid
	myTextGrid_part = Extract part... part_start part_end no

	nIntervals = Get number of intervals... target_tier

	for iInterval to nIntervals
		select myTextGrid_part
		label$ = Get label of interval... target_tier iInterval
		label$ =  replace$(label$, " ","",0)
		if label$ = target_label$
			nTargets += 1
			call treat_word
		endif
	endfor

	select mySound_part


	if iPart = 1
		Save as 'output_format$' file... 'folder$'/'anonymised_sound$'
	else
		Append to existing sound file... 'folder$'/'anonymised_sound$'
	endif
	
	anon$ = "anonymised words"

	if nTargets = 1
		anon$ = anon$ - "s"
	endif

	if verbose
		printline 'tab$'• segment 'iPart' ['part_start:3'..'part_end:3'] 'nTargets' 'anon$'.
	endif

	plus myTextGrid_part
	Remove
endproc

procedure treat_word
	if automatic_max_and_min
		select mySound_part
		call calculate_min_max_f0
	else
		min_f0 = minimum_f0
		max_f0 = maximum_f0
	endif

####### initailise booleans for left or right parts
	left = 1
	right = 1
#######

	select myTextGrid_part
	word_start = Get start point... target_tier iInterval
	if word_start = 0
		left = 0
	endif

	word_end = Get end point... target_tier iInterval
	if word_end = section
		right = 0
	endif

	select mySound_part


	if left
		myLeft = Extract part... 0 word_start rectangular 1 no
	endif

	select mySound_part
	myWord = Extract part... word_start word_end rectangular 1 no
	myScale = Get intensity (dB)
	myPitch = To Pitch... timestep min_f0 max_f0
	myHum_temp = To Sound (hum)
	myHum = Resample... sampling_frequency precision
	select myHum_temp
	Remove
	select myHum

	if nChannels = 2
		myHum_temp = Convert to stereo
		select myHum
		Remove
		myHum = myHum_temp
	endif

	select myWord
	myIntensity = To Intensity... min_f0 timestep no
	myIntensityTier = Down to IntensityTier
	plus myHum
	myNewHum = Multiply... yes
	if myScale != undefined
		Scale intensity... myScale
	endif

	select mySound_part
	if right
		myRight = Extract part... word_end section rectangular 1 no
	endif

	if left
		select myLeft
		plus myNewHum
	else
		select myNewHum
	endif

	if right
		plus myRight
	endif

	myNew_part = Concatenate
	select mySound_part
	if left
		plus myLeft
	endif
	plus myWord
	plus myPitch
	plus myHum
	plus myIntensity
	plus myIntensityTier
	plus myNewHum
	if right
		plus myRight
	endif
	Remove
	mySound_part = myNew_part
endproc

procedure calculate_min_max_f0
#  estimate of newMaxF0 as 2.5 * quantile 0.75
#  and newMinF0 as 0.5 * quantile 0.25
#  rounded to higher (resp. lower) 10
	To Pitch... 'timestep' 'minimum_f0' 'maximum_f0'
	.q75 = Get quantile... 0.0 0.0 0.75 Hertz
	.q25 = Get quantile... 0.0 0.0 0.25 Hertz
	max_f0 = 10*ceiling((2.5*.q75)/10)
	min_f0 = 10*floor((0.75*.q25)/10)
	Remove
endproc

#Version history

# [2013-03-07] remove leading or trailing spaces from labels
#			the folder containing the Sounds and TextGrids can be specified or selected with the browser
# [2013-03-06] corrected bug when label to anonymise crosses boundary of section
#			allow output in WAV, AIFF, AIFC, Next/SUN or NIST format
# [2011-03-25] changed extensions ".textGrid" to ".TextGrid"
#			changed max_f0 to 2.5 quantile 75 to allow for more expressive speech
#			resampled buzz to sampling rate of original sound
# [2010:05:24] allowed possibility of stereo long sounds
# [2008:05:25]	first version working.

