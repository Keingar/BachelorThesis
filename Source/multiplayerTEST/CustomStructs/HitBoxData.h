// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollisionShape.h"
#include "HitBoxData.generated.h"
/**
 * 
 */

UENUM(BlueprintType)
enum class ECustomCollisionShape : uint8
{
    Box     UMETA(DisplayName = "Box"),
    Sphere  UMETA(DisplayName = "Sphere"),
    Capsule UMETA(DisplayName = "Capsule")
};


USTRUCT(BlueprintType)
struct FHitBoxData
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    ECustomCollisionShape ShapeType = ECustomCollisionShape::Capsule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox", meta = (EditCondition = "ShapeType == ECustomCollisionShape::Box", EditConditionHides))
    FVector BoxExtent = FVector(50.0f, 50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox", meta = (EditCondition = "ShapeType == ECustomCollisionShape::Sphere || ShapeType == ECustomCollisionShape::Capsule", EditConditionHides))
    float Radius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox", meta = (EditCondition = "ShapeType == ECustomCollisionShape::Capsule", EditConditionHides))
    float CapsuleHalfHeight = 100.0f;
	
};
