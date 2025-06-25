#include "wrapper/fdcan.hpp"

bool start_can(FDCAN_HandleTypeDef* hfdcan)
{
	static FDCAN_FilterTypeDef sFilterConfig =
	{
		.IdType = FDCAN_STANDARD_ID,
		.FilterIndex = 0,
		.FilterType = FDCAN_FILTER_MASK,
		.FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
		.FilterID1 = 0,
		.FilterID2 = 0
	};

	const auto can_notification_status = HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	if (HAL_OK != can_notification_status)
	{
		return false;
	}

	const auto can_filter_config_status = HAL_FDCAN_ConfigFilter(hfdcan, &sFilterConfig);

	if (HAL_OK != can_filter_config_status)
	{
		return false;
	}

	if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
	{
		return false;
	}
	return true;
}