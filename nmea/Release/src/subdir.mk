################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/context.c \
../src/generate.c \
../src/generator.c \
../src/gmath.c \
../src/info.c \
../src/nmeatime.c \
../src/parse.c \
../src/parser.c \
../src/sentence.c \
../src/tok.c 

OBJS += \
./src/context.o \
./src/generate.o \
./src/generator.o \
./src/gmath.o \
./src/info.o \
./src/nmeatime.o \
./src/parse.o \
./src/parser.o \
./src/sentence.o \
./src/tok.o 

C_DEPS += \
./src/context.d \
./src/generate.d \
./src/generator.d \
./src/gmath.d \
./src/info.d \
./src/nmeatime.d \
./src/parse.d \
./src/parser.d \
./src/sentence.d \
./src/tok.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../nmea/include" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


