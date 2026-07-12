// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilTextFunctionLibrary.h"

bool UDirectiveUtilTextFunctionLibrary::IsNotEmpty(const FText& Text)
{
	return !Text.IsEmpty();
}
