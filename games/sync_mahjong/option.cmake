target_link_libraries(sync_mahjong Mahjong MahjongAlgorithm calsht_dw)
add_dependencies(sync_mahjong Mahjong MahjongAlgorithm calsht_dw)
if (WITH_TEST)
    target_link_libraries(test_game_sync_mahjong Mahjong MahjongAlgorithm calsht_dw)
    add_dependencies(test_game_sync_mahjong Mahjong MahjongAlgorithm calsht_dw)
    target_link_libraries(run_game_sync_mahjong Mahjong MahjongAlgorithm calsht_dw)
    add_dependencies(run_game_sync_mahjong Mahjong MahjongAlgorithm calsht_dw)
endif()
