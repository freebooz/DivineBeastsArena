// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"
#include "GameDBA/Frontend/Online/DBAApiClientSubsystem.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAApiClientSingleFlightRefreshTest,
	"DBA.Frontend.ApiClient.SingleFlightRefresh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAApiClientSingleFlightRefreshTest::RunTest(const FString& Parameters)
{
	UDBAApiClientSubsystem* ApiClient = NewObject<UDBAApiClientSubsystem>();
	UObject* CallbackOwner = NewObject<UObject>();
	int32 TransportCallCount = 0;
	int32 RefreshCallCount = 0;
	int32 CompletionCount = 0;
	TFunction<void(bool)> DeferredRefreshCompletion;

	ApiClient->SetMockTransportForTests([&TransportCallCount](const FDBAApiRequest&, TFunction<void(const FDBAApiResponse&)> Complete)
	{
		++TransportCallCount;
		FDBAApiResponse Response;
		if (TransportCallCount <= 10)
		{
			Response.HttpStatusCode = 401;
			Response.Result = FDBAOperationResult::Failure(UDBAFrontendErrorMapper::FromHttpStatus(401, TEXT("auth.expired")));
		}
		else
		{
			Response.Result = FDBAOperationResult::Success();
		}
		Complete(Response);
	});
	ApiClient->SetMockRefreshForTests([&RefreshCallCount, &DeferredRefreshCompletion](TFunction<void(bool)> Complete)
	{
		++RefreshCallCount;
		DeferredRefreshCompletion = MoveTemp(Complete);
	});

	for (int32 Index = 0; Index < 10; ++Index)
	{
		ApiClient->Get(TEXT("/api/frontend/test"), true, CallbackOwner, [&CompletionCount](const FDBAApiResponse& Response)
		{
			if (Response.Result.bSuccess)
			{
				++CompletionCount;
			}
		});
	}

	TestEqual(TEXT("十个并发 401 只应发起一次刷新"), RefreshCallCount, 1);
	TestTrue(TEXT("刷新回调应已被登记"), static_cast<bool>(DeferredRefreshCompletion));
	DeferredRefreshCompletion(true);
	TestEqual(TEXT("刷新成功后十个请求都应完成"), CompletionCount, 10);
	TestEqual(TEXT("十个初始请求与十个重试请求均应走 Mock 传输"), TransportCallCount, 20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAApiClientCancellationTest,
	"DBA.Frontend.ApiClient.CancelledOwnerDoesNotReceiveCallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAApiClientCancellationTest::RunTest(const FString& Parameters)
{
	UDBAApiClientSubsystem* ApiClient = NewObject<UDBAApiClientSubsystem>();
	UObject* CallbackOwner = NewObject<UObject>();
	TFunction<void(const FDBAApiResponse&)> DeferredResponse;
	int32 CompletionCount = 0;

	ApiClient->SetMockTransportForTests([&DeferredResponse](const FDBAApiRequest&, TFunction<void(const FDBAApiResponse&)> Complete)
	{
		DeferredResponse = MoveTemp(Complete);
	});

	const FGuid RequestId = ApiClient->Get(TEXT("/api/frontend/cancel"), false, CallbackOwner, [&CompletionCount](const FDBAApiResponse&)
	{
		++CompletionCount;
	});
	TestTrue(TEXT("已登记请求可以取消"), ApiClient->CancelRequest(RequestId));
	TestTrue(TEXT("Mock 传输应已保留完成回调"), static_cast<bool>(DeferredResponse));
	FDBAApiResponse Response;
	Response.Result = FDBAOperationResult::Success();
	DeferredResponse(Response);
	TestEqual(TEXT("取消后的请求不得更新原页面"), CompletionCount, 0);
	return true;
}

#endif
