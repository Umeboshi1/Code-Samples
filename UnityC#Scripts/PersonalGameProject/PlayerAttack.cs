using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerAttack : MonoBehaviour
{
    //attack hitboxes
    public GameObject LeftHit;
    public GameObject RightHit;
    public GameObject UpHit;
    public GameObject DownHit;

    //wait time between attacks
    public float attackTime;
    public float attackTimeLeft;

    //buffer input for the Y button
    float yBuffer;
    public float yBufferTime;

    //object to throw
    public GameObject spinThrow;

    PlayerMovement playerMov;

    public float spinChargeTime;
    float currCharge;
    bool spinCharging;

    // Start is called before the first frame update
    void Start()
    {
        playerMov = GetComponent<PlayerMovement>();
        spinCharging = false;
    }

    // Update is called once per frame
    void Update()
    {
        float horizontal = Input.GetAxis("LeftJoystickHorizontal");
        float vertical = Input.GetAxis("LeftJoystickVertical");
        if (Input.GetButtonDown("XButton") && attackTimeLeft < 0.0f)
        {
            if (vertical < -.75)
            {
                UpHit.SetActive(true);
                attackTimeLeft = attackTime;
                checkHit(UpHit);
            } else if (vertical > .75)
            {
                DownHit.SetActive(true);
                attackTimeLeft = attackTime;
                checkHit(DownHit);
            } else if (playerMov.isFacingRight)
            {
                RightHit.SetActive(true);
                attackTimeLeft = attackTime;
                checkHit(RightHit);
            } else
            {
                LeftHit.SetActive(true);
                attackTimeLeft = attackTime;
                checkHit(LeftHit);
            }
        }

        //check if still attacking
        attackTimeLeft -= Time.deltaTime;
        if(attackTimeLeft < 0.0f)
        {
            RightHit.SetActive(false);
            LeftHit.SetActive(false);
            UpHit.SetActive(false);
            DownHit.SetActive(false);
        }


        if (Input.GetButtonDown("YButton"))
        {
            yBuffer = yBufferTime;
        }

        if(yBuffer > 0)
        {
            //spin throw
            yBuffer = -1f;
            spinCharging = true;
            currCharge = 0;
        }

        if (spinCharging)
        {
            if (Input.GetButtonUp("YButton") || currCharge > spinChargeTime)
            {
                GameObject spin = Instantiate(spinThrow, transform);
                spin.GetComponent<SpinThrow>().initialVelocity(playerMov.isFacingRight, Mathf.Min((currCharge/spinChargeTime), 1.0f));
                spinCharging = false;
            }
            yBuffer -= Time.deltaTime;
            currCharge += Time.deltaTime;
        }
    }

    //check all hitboxes of given direction
    private void checkHit(GameObject hitboxDir)
    {
        Collider[] playerAttackBoxes = hitboxDir.GetComponentsInChildren<BoxCollider>();
        //Debug.Log(playerAttackBoxes.Length);
        foreach(Collider col in playerAttackBoxes)
        {
            Collider[] hits;
            hits = Physics.OverlapBox(col.bounds.center,col.bounds.extents);

            //Debug.Log("Objects hit: " + hits.Length);
            List<string> colliderNames = new List<string>();
            foreach(Collider hit in hits)
            {
                if (!colliderNames.Contains(hit.name))
                {
                    if (hit.tag == "Enemy")
                    {
                        Debug.Log("Enemy Hit");
                        hit.GetComponent<EnemyHealth>().takeHit(5.0f);
                        playerMov.rebound(hitboxDir);
                    }

                    if (hit.tag == "CanHit")
                    {
                        playerMov.rebound(hitboxDir);
                        hit.gameObject.GetComponent<SpinThrow>().gotHit(hitboxDir);
                    }
                } else
                {
                    colliderNames.Add(hit.name);
                }
            }
        }
    }
}
