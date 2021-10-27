Feature: change pass phrase
  In order to conform OpenPGP card 2.0 specification
  A token should support pass phrase: PW1, PW3 and reset code

  Scenario: change PW1 (in admin-less mode)
     Given cmd_change_reference_data with 1 and "another user pass phrasePASSPHRASE SHOULD BE LONG"
     Then it should get success

  Scenario: verify PW1 (1) again
     Given cmd_verify with 1 and "PASSPHRASE SHOULD BE LONG"
     Then it should get success

  Scenario: verify PW1 (2) again
     Given cmd_verify with 2 and "PASSPHRASE SHOULD BE LONG"
     Then it should get success

  Scenario: verify PW3 (admin-less mode)
     Given cmd_verify with 3 and "PASSPHRASE SHOULD BE LONG"
     Then it should get success

  Scenario: setup reset code again (in admin-less mode)
     Given cmd_put_data with d3 and "example reset code 000"
     Then it should get success

  Scenario: reset pass phrase by reset code (in admin-less mode)
     Given cmd_reset_retry_counter with 0 and "example reset code 000new user pass phrase"
     Then it should get success

  Scenario: verify PW1 (1) again
     Given cmd_verify with 1 and "new user pass phrase"
     Then it should get success

  Scenario: verify PW1 (2) again
     Given cmd_verify with 2 and "new user pass phrase"
     Then it should get success

  Scenario: change PW1
     Given cmd_change_reference_data with 1 and "new user pass phraseanother user pass phrase"
     Then it should get success

  Scenario: verify PW1 (1) again
     Given cmd_verify with 1 and "another user pass phrase"
     Then it should get success

  Scenario: verify PW1 (2) again
     Given cmd_verify with 2 and "another user pass phrase"
     Then it should get success

  Scenario: verify PW3 (admin-less mode)
     Given cmd_verify with 3 and "another user pass phrase"
     Then it should get success
