################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../device.c \
../do_file.c \
../do_format.c \
../do_playback.c \
../do_preview.c \
../do_record.c \
../do_setting.c \
../do_takephoto.c \
../h264.c \
../jpeg.c \
../main.c \
../msg.c 

OBJS += \
./device.o \
./do_file.o \
./do_format.o \
./do_playback.o \
./do_preview.o \
./do_record.o \
./do_setting.o \
./do_takephoto.o \
./h264.o \
./jpeg.o \
./main.o \
./msg.o 

C_DEPS += \
./device.d \
./do_file.d \
./do_format.d \
./do_playback.d \
./do_preview.d \
./do_record.d \
./do_setting.d \
./do_takephoto.d \
./h264.d \
./jpeg.d \
./main.d \
./msg.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/jyin/eclipse-workspace/cvr-slave/libs/x264" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


