# Peyment flow diagram

## 1. Peyment

```
graph TD
A[invoke_preflight]-->B[Payment::preflight]
```


## 2. applySteps



```
graph TD

4A[NetworkOPsImp::checkLastClosedLedger]
3A1[NetworkOPsImp::switchLastClosedLedger]
3A2[LedgerConsensusImp<Traits>::accept]
2A[TxQ::accept]
1A[TxQ::MaybeTx::apply]
A[preflight]
B[preclaim]
C[calculateBaseFee]
D[calculateConsequences]
E[doApply]

10A[LedgerConsensusImp<Traits>::timerEntry]-->13A
13A[LedgerConsensusImp<Traits>::checkLCL]-->12A
12A[LedgerConsensusImp<Traits>::handleLCL]-->11A1
11A1[LedgerConsensusImp<Traits>::startRound]-->10A

12A2[NetworkOPsImp::onDeadlineTimer]-->11A2
11A2[NetworkOPsImp::processHeartbeatTimer]-->10A
10A[LedgerConsensusImp<Traits>::timerEntry]-->9A1
9A1[LedgerConsensusImp<Traits>::stateEstablish]-->8A
9A2[LedgerConsensusImp<Traits>::simulate]-->8A
8A[LedgerConsensusImp<Traits>::beginAccept]-->3A2
3A2-->7A
7A[LedgerConsensusImp<Traits>::endConsensus]-->6A[NetworkOPsImp::endConsensus]
6A-->5A
5A[NetworkOPsImp::tryStartConsensus]-->4A
4A-->3A1
3A1-->2A
3A2-->2A
2A-->1A
1A-->A
A-->B
B-->C
C-->D
D-->E
```
## 3. How the txs are moved from the queue to the new open ledger.

```
graph TD

A(start)
B{`featureFeeEscalation` enabled?}
C[Iterate over the txs from highest fee level to lowest.]
D[For each tx:]
D1{the first tx in the queue for this account?}
D11[Skip this tx. We'll come back to it later.]
D12{tx fee level < current required}
E(Stop)

A-->B
B-->|Yes|C
B-->|No|E
C-->D
D-->D1
D1-->|Yes|D11
D1-->|No|D12
```


## 4. How the decision to apply, queue, or reject is made:

```
/*
    How the decision to apply, queue, or reject is made:
    0. Is `featureFeeEscalation` enabled?
        Yes: Continue to next step.
        No: Fallback to `mtchain::apply`. Stop.
    1. Does `preflight` indicate that the tx is valid?
        No: Return the `TER` from `preflight`. Stop.
        Yes: Continue to next step.
    2. Is there already a tx for the same account with the
            same sequence number in the queue?
        Yes: Is `txn`'s fee `retrySequencePercent` higher than the
                queued transaction's fee? And is this the last tx
                in the queue for that account, or are both txs
                non-blockers?
            Yes: Remove the queued transaction. Continue to next
                step.
            No: Reject `txn` with `telINSUF_FEE_P` or
                `telCAN_NOT_QUEUE`. Stop.
        No: Continue to next step.
    3. Does this tx have the expected sequence number for the
            account?
        Yes: Continue to next step.
        No: Are all the intervening sequence numbers also in the
                queue?
            No: Continue to the next step. (We expect the next
                step to return `terPRE_SEQ`, but won't short
                circuit that logic.)
            Yes: Is the fee more than `multiTxnPercent` higher
                    than the previous tx?
                No: Reject with `telINSUF_FEE_P`. Stop.
                Yes: Are any of the prior sequence txs blockers?
                    Yes: Reject with `telCAN_NOT_QUEUE`. Stop.
                    No: Are the fees in-flight of the other
                            queued txs >= than the account
                            balance or minimum account reserve?
                        Yes: Reject with `telCAN_NOT_QUEUE`. Stop.
                        No: Create a throwaway sandbox `View`. Modify
                            the account's sequence number to match
                            the tx (avoid `terPRE_SEQ`), and decrease
                            the account balance by the total fees and
                            maximum spend of the other in-flight txs.
                            Continue to the next step.
    4. Does `preclaim` indicate that the account is likely to claim
            a fee (using the throwaway sandbox `View` created above,
            if appropriate)?
        No: Return the `TER` from `preclaim`. Stop.
        Yes: Continue to the next step.
    5. Did we create a throwaway sandbox `View`?
        Yes: Continue to the next step.
        No: Is the `txn`s fee level >= the required fee level?
            Yes: `txn` can be applied to the open ledger. Pass
                it to `doApply()` and return that result.
            No: Continue to the next step.
    6. Can the tx be held in the queue? (See TxQ::canBeHeld).
            No: Reject `txn` with `telINSUF_FEE_P` if this tx
                has the current sequence, or `telCAN_NOT_QUEUE`
                if not. Stop.
            Yes: Continue to the next step.
    7. Is the queue full?
        No: Continue to the next step.
        Yes: Is the `txn`'s fee level higher than the end /
                lowest fee level item's fee level?
            Yes: Remove the end item. Continue to the next step.
            No: Reject `txn` with a low fee TER code.
    8. Put `txn` in the queue.
*/
```
